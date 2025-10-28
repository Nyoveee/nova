#include <utility>
#include <ranges>
#include <functional>
#include <algorithm>

#include "animatorController.h"

#include "imgui.h"
#include "imgui_node_editor.h"
#include "IconsFontAwesome6.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"
#include "InputManager/inputManager.h"

#include "component.h"
#include "controller.h"

#include "nova_math.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

namespace ed = ax::NodeEditor;

AnimatorController::AnimatorController(Editor& editor) :
	editor				{ editor },
	resourceManager		{ editor.resourceManager },
	assetManager		{ editor.assetManager },
	context				{ nullptr }
{
	context = ed::CreateEditor(nullptr);	
}

AnimatorController::~AnimatorController() {
	ed::DestroyEditor(context);
}

void AnimatorController::update() {
	ImGui::Begin(ICON_FA_OBJECT_GROUP " Animator");

	// ===================================================================================
	// We attempt to retrieve controllerfrom the animator component of the currently selected entity.
	// ===================================================================================
	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	entt::entity selectedEntity = editor.getSelectedEntities()[0];

	Animator* animator = editor.engine.ecs.registry.try_get<Animator>(selectedEntity);

	if (!animator) {
		ImGui::Text("Selected entity has no animator component.");
		ImGui::End();
		return;
	}

	auto&& [controller, loadStatus] = resourceManager.getResource<Controller>(animator->controllerId);

	if (!controller) {
		switch (loadStatus)
		{
		case ResourceManager::QueryResult::Invalid:
			ImGui::Text("Animator is pointing to an invalid animation controller asset.");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::WrongType:
			ImGui::Text("This should never happened. Resource ID is not a controller?");
			assert(false && "Resource ID is not a controller.");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::Loading:
			ImGui::Text("Loading..");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::LoadingFailed:
			ImGui::Text("Loading of model failed.");
			ImGui::End();
			return;
		default:
			assert(false);
			ImGui::End();
			return;
		}
	}

	displayLeftPanel(*animator, *controller);
	handleDragAndDrop(*controller);

	ImGui::SameLine();
	displayRightPanel(*animator, *controller);

	ImGui::End();
}

void AnimatorController::displayLeftPanel(Animator& animator, Controller& controller) {
	ImGui::BeginChild("Left Panel", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
	
	ed::SetCurrentEditor(context);

	// Start of the node editor..
	ed::Begin("Node Editor", ImVec2(0.0f, 0.0f));

	// 1) Render all the nodes..
	renderNodes(animator, controller);

	// 2) Render all the links..
	renderNodeLinks(controller);

	// Update selected nodes.. such a bad api not gonna lie..
	selectedNodes.resize(ed::GetSelectedObjectCount());
	int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
	selectedNodes.resize(nodeCount);

	// Update selected links.. such a bad api not gonna lie..
	selectedLinks.resize(ed::GetSelectedObjectCount());
	int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));
	selectedLinks.resize(linkCount);

	// 3) Handle interactions
	if (ed::BeginCreate()) {
		handleNodeLinking(controller);
	}
	ed::EndCreate(); // Wraps up object creation action handling.

	// 4) Handle deletion action, only when not in simulation mode.
	if (!editor.engine.isInSimulationMode() && ed::BeginDelete()) {
		handleDeletion(controller);
	}

	ed::EndDelete();

	ed::End();

	ImGui::EndChild();
}

void AnimatorController::displayRightPanel(Animator& animator, Controller& controller) {
	ImGui::BeginChild("Right Panel");

	if (ImGui::Button("Center all node position")) {

		for (auto&& [nodeId, _] : controller.data.nodes) {
			ed::NodeId edNodeId = static_cast<std::size_t>(nodeId);
			ed::SetNodePosition(edNodeId, { 0.f, 0.f });
		}

		ed::NavigateToContent(0.0f);
	}

	if (ImGui::BeginTabBar("TabBar")) {
		if (ImGui::BeginTabItem("Inspector")) {
			displayParameterWindow(animator, controller);
			displaySelectedNodeProperties(animator, controller);

			ImGui::EndTabItem();
		}

		int imguiCounter = 0;

		if (ImGui::BeginTabItem("Debug")) {
			for (auto&& [nodeId, node] : controller.data.nodes) {
				ed::NodeId edNodeId = static_cast<std::size_t>(nodeId);
				auto pos = ed::GetNodePosition(edNodeId);
				ImGui::PushID(imguiCounter);

				ImGui::Text("Node: %s, x: %f, y: %f", node.name.c_str(), pos.x, pos.y);

				if (ImGui::Button("Center button to screen")) {
					ed::CenterNodeOnScreen(edNodeId);
				}

				ImGui::PopID();
				++imguiCounter;
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::EndChild();
}

void AnimatorController::displayParameterWindow(Animator& animator, Controller& controller) {
	ImGui::BeginChild("Parameter Window", {}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

	// In simulation mode, every animator component will have a copy of the parameters.
	// In non simulation mode, we are actually viewing at the controller's parameter. 
	// Very cool. Let's utilise IILE to construct our parameter container based on condition.
	auto&& parameters = [&]() -> std::vector<Controller::Parameter>& {
		if (editor.isInSimulationMode()) {
			return animator.parameters;
		}
		else {
			return controller.data.parameters;
		}
	}();

	static int counter = 0;

	// Display combo box to add parameter
	if (ImGui::BeginCombo("##", "[+] Add Parameter")) {
		std::string name = Logger::getUniqueTimedId();

		if (ImGui::Selectable("Boolean")) {
			parameters.push_back({ name + std::to_string(counter++), false });
		}

		if (ImGui::Selectable("Integer")) {
			parameters.push_back({ name + std::to_string(counter++), 0 });
		}

		if (ImGui::Selectable("Float")) {
			parameters.push_back({ name + std::to_string(counter++), 0.f });
		}

		ImGui::EndCombo();
	}

	//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	// Display table to list all the parameters..
	if (ImGui::BeginTable("Parameters Table", 4)) {
		// Setup columns (optional, but good practice for headers and sizing)
		ImGui::TableSetupColumn("Type",    ImGuiTableColumnFlags_WidthFixed, 40.0f);
		ImGui::TableSetupColumn("Name",    ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Delete?", ImGuiTableColumnFlags_WidthFixed, 40.0f);

		// ImGui::TableHeadersRow(); 

		// Populate table rows
		int imguiCounter = 0;
		for (auto it = parameters.begin(); it != parameters.end();) {
			ImGui::PushID(imguiCounter++);

			auto&& [name, variant] = *it;

			ImGui::TableNextRow(); // Start a new row

			// =================================================================
			// Column 1: Handle display of type of parameter.
			// =================================================================
			const char* typeInText = "We live in a world full of lies.";

			std::visit([&](auto&& parameter) {
				using ParameterType = std::decay_t<decltype(parameter)>;

				if constexpr (std::same_as<bool, ParameterType>) {
					typeInText = "bool";
				}
				else if constexpr (std::same_as<int, ParameterType>) {
					typeInText = "int";
				}
				else if constexpr (std::same_as<float, ParameterType>) {
					typeInText = "float";

				}
				else {
					[&] <bool flag = false, typename T = ParameterType>() {
						static_assert(flag, "Not all types of variant accounted for." __FUNCSIG__);
					}();
				}
			}, variant);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text(typeInText);

			// =================================================================
			// Column 2: Handle parameter name and name change.
			// =================================================================
			// We first check if the temporary map contains this current parameter name.
			auto iterator = temporaryNames.find(name);

			// Doesn't have it, we add a new entry..
			if (iterator == temporaryNames.end()) {
				temporaryNames.insert({ name, name });
			}

			// We edit this temporary name..
			std::string& temporaryName = temporaryNames.at(name);
			ImGui::TableSetColumnIndex(1);
			ImGui::InputText("##name", &temporaryName);

			// We modify the parameter's actual name by modifying the unordered_map entry.
			// We only do this if the new name is valid.	
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				// A name is valid if it's a unique name in the unordered_map.
				auto parameterIt = std::ranges::find_if(parameters, [&](auto&& element) { return element.name == temporaryName; });

				if (parameterIt != parameters.end()) {
					// invalid name, let's reset our temporary name.
					temporaryName = name;
				}
				else {
					name = temporaryName;
				}
			}

			// =================================================================
			// Column 3: Handle variant value..
			// =================================================================
			ImGui::TableSetColumnIndex(2);
			DisplayProperty<std::decay_t<decltype(variant)>>(editor, "##value", variant);

			// =================================================================
			// Column 4: Handle parameter deletion..
			// =================================================================
			ImGui::TableSetColumnIndex(3);

			if (ImGui::Button("[-]")) {
				it = parameters.erase(it);
			}
			else {
				++it;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	ImGui::EndChild();
}

void AnimatorController::displaySelectedNodeProperties([[maybe_unused]] Animator& animator, Controller& controller) {
	auto&& nodes = controller.data.nodes;

	if (selectedNodes.empty()) {
		ImGui::Text("No node selected.");
		return;
	}

	ImGui::BeginChild("Transition Window", {}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

	for (auto&& edNodeId : selectedNodes) {
		ControllerNodeID nodeId = static_cast<ControllerNodeID>(static_cast<std::size_t>(edNodeId));
		auto iterator = nodes.find(nodeId);
		
		if (iterator == nodes.end()) {
			continue;
		}

		auto&& [_, node] = *iterator;

		ImGui::SeparatorText(node.name.c_str());
		// ImGui::Text("ID: %zu", static_cast<std::size_t>(edNodeId));

		ImGui::Checkbox("To loop?", &node.toLoop);

		if (node.transitions.empty()) {
			ImGui::Text("No transition. Create a transition in the node graph!");
		}
		else {
			std::function<void()> swapTransitionOrder = nullptr;

			int transitionIndex = 0;

			for (Controller::Transition& transition : node.transitions) {
				auto nextIterator = nodes.find(transition.nextNode);

				if (nextIterator == nodes.end()) {
					continue;
				}

				ImGui::PushID(transitionIndex);

				auto&& [__, nextNode] = *nextIterator;

				std::string transitionText = "[" + std::to_string(transitionIndex) +  "] Transition: '" + node.name + "' >> '" + nextNode.name + "'";
				
				bool isTreeNodeActive = ImGui::TreeNode(transitionText.c_str());
				
				// Drag and drop to swap transition..
				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("SWAP_TRANSITION", &transitionIndex, sizeof(int));
					ImGui::EndDragDropSource();
				}

				// Drop Target
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SWAP_TRANSITION")) {
						int fromTransitionIndex = *(const int*)payload->Data;

						swapTransitionOrder = [&, fromIndex = fromTransitionIndex, toIndex = transitionIndex]() {
							std::swap(node.transitions[fromIndex], node.transitions[toIndex]);
						};
					}
					ImGui::EndDragDropTarget();
				}

				if (isTreeNodeActive) {
					if (transition.conditions.empty()) {
						ImGui::Text("No conditions. This transition will always happen.");
					}
					else if (ImGui::BeginTable("Condition Table", 4, ImGuiTableFlags_Borders)) {
						ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Check", ImGuiTableColumnFlags_WidthFixed, 120.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 30.f);

						int transitionCounter = 1;

						for (auto it = transition.conditions.begin(); it != transition.conditions.end();) {
							Controller::Condition& condition = *it;

							ImGui::PushID(transitionCounter++);
							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							displayParameterComboBox(controller, condition);

							ImGui::TableNextColumn();

							if (std::holds_alternative<bool>(condition.value)) {
								ImGui::Text("==");
							}
							else {
								DisplayProperty<Controller::Condition::Check>(editor, "##check", condition.check);
							}

							ImGui::TableNextColumn();
							DisplayProperty<decltype(condition.value)>(editor, "##value", condition.value);

							ImGui::TableNextColumn();

							if (ImGui::Button("[-]")) {
								it = transition.conditions.erase(it);
							}
							else {
								++it;
							}

							ImGui::PopID();
						}

						ImGui::EndTable();
					}

					if (ImGui::Button("[+] Add condition")) {
						createNewCondition(controller, transition);
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
				transitionIndex++;
			}

			if (swapTransitionOrder) {
				swapTransitionOrder();
			}
		}
	}

	ImGui::EndChild();
}

void AnimatorController::handleDragAndDrop(Controller& controller) {
	if (!ImGui::BeginDragDropTarget()) {
		return;
	}

	ImGuiPayload const* payload = nullptr;
	if (payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM"), !payload) {
		return;
	}

	auto&& [animationId, name] = *((std::pair<std::size_t, const char*>*)payload->Data);

	// only handle it if it's an animation.
	if (!editor.resourceManager.isResource<Model>(animationId)) {
		return;
	}

	ControllerNodeID nodeId = Math::getGUID();

	controller.data.nodes.insert({ nodeId, Controller::Node {
		nodeId,
		animationId,
		{},
		true,
		name
	}});
}

void AnimatorController::renderNodes(Animator& animator, Controller& controller) {
	pinMetaData.clear();
	nodeToPins.clear();

	int counter = 1;

	// Render all nodes..
	for (auto&& [id, node] : controller.data.nodes) {
		ed::NodeId nodeId = static_cast<std::size_t>(id);
		ed::PinId inPinId = counter++;
		ed::PinId outPinId = counter++;

		std::string animationName;

		if (id == ENTRY_NODE) {
			animationName = node.name;
		}
		else {
			// Get animation name..
			auto* descriptor = assetManager.getDescriptor(node.animation);

			if (!descriptor) {
				continue;
			}

			animationName = descriptor->name;
		}

		ImGui::PushID(static_cast<int>(static_cast<std::size_t>(id)));

		// Highlight the currently selected node.
		if (animator.currentNode == id && animator.currentNode != ENTRY_NODE) {
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4{ 1.f, 1.f, 0.f, 1.f });
			ed::PushStyleColor(ed::StyleColor_NodeBg,	  ImVec4{ 0.2f, 0.5f, 0.5f, 0.5f });
		}

		// Render a node..
		ed::BeginNode(nodeId);
		ImGui::Text("%s", animationName.c_str());

		if (id != ENTRY_NODE) {
			ed::BeginPin(inPinId, ed::PinKind::Input);
			ImGui::Text("-> In");
			ed::EndPin();
		
			ImGui::SameLine();
		}

		ed::BeginPin(outPinId, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();

		// Render progress bar..
		// Attempts to get animation data from the current node..
		auto&& [animation, _] = resourceManager.getResource<Model>(node.animation);

		if (animation && animation->animations.size()) {
			float percentage = animator.currentNode == id ? animator.timeElapsed / animation->animations[0].durationInSeconds : 0.f;
			ImGui::ProgressBar(percentage, ImVec2{ 100.f, 20.f });
		}

		ed::EndNode();

		// Highlight the currently selected node, pop style.
		if (animator.currentNode == id && animator.currentNode != ENTRY_NODE) {
			ed::PopStyleColor(2);
		}

		ImGui::PopID();

		// save mapping from pin to nodes..
		std::pair<std::size_t, PinData> pairIn { static_cast<std::size_t>(inPinId),  PinData{ id, PinData::Type::In } };
		std::pair<std::size_t, PinData> pairOut{ static_cast<std::size_t>(outPinId), PinData{ id, PinData::Type::Out } };

		pinMetaData.insert(pairIn);
		pinMetaData.insert(pairOut);

		// save mapping from nodes to pins..
		nodeToPins.insert({ id, Pins{ inPinId, outPinId } });
	}
}

void AnimatorController::renderNodeLinks(Controller& controller) {
	auto&& nodes = controller.data.nodes;

	int linkCounter = 1;

	// [data cleaning]
	// we first iterate through every node and remove invalid transitions..
	for (auto&& [nodeId, node] : nodes) {

		for (auto it = node.transitions.begin(); it != node.transitions.end();) {
			auto& transition = *it;

			auto iterator = nodes.find(transition.nextNode);

			// a transition is invalid if it's next node is invalid.
			if (iterator == nodes.end()) {
				it = node.transitions.erase(it);
			}
			else {
				++it;
			}
		}
	}

	linksToTransition.clear();

	// for every node..
	for (auto&& [nodeId, node] : nodes) {
		int transitionIndex = 0;

		// iterate through the node's transitions..
		for (auto&& transition : node.transitions) {
			// we retrieve details about the next node
			auto iterator = nodes.find(transition.nextNode);
			auto&& [nextNodeId, nextNode] = *iterator;	// we have cleaned the data in the previous loop, safe deference.

			// get the respective out and in pins..
			auto&& [_, outPin] = nodeToPins.at(nodeId);
			auto&& [inPin, __] = nodeToPins.at(nextNodeId);

			ed::LinkId linkId = linkCounter++;

			// create link..
			ed::Link(linkId, outPin, inPin);

			linksToTransition.insert({ static_cast<std::size_t>(linkId), { nodeId, transitionIndex } });
			++transitionIndex;
		}
	}
}

void AnimatorController::handleNodeLinking(Controller& controller) {
	auto&& nodes = controller.data.nodes;

	// Handle creation action, returns true if editor want to create new object (node or link)
	ed::PinId inPinId, outPinId;

	// QueryNewLink returns true if editor want to create new link between pins.
	if (!ed::QueryNewLink(&outPinId, &inPinId)) {
		return;
	}

	// no valid connect made yet..
	if (!inPinId || !outPinId) {
		return;
	}

	auto&& inPinIterator  = pinMetaData.find(static_cast<std::size_t>(inPinId));
	auto&& outPinIterator = pinMetaData.find(static_cast<std::size_t>(outPinId));

	// these iterators are valid..
	if (inPinIterator != pinMetaData.end() && outPinIterator != pinMetaData.end()) {
		auto&& [_,		   pinInData]	= *inPinIterator;
		auto&& [__,		   pinOutData]	= *outPinIterator;
		auto&& [nodeInId,  pinInType]	= pinInData;
		auto&& [nodeOutId, pinOutType]	= pinOutData;

		// pins are not in and out respectively.. reject!
		if (pinInType != PinData::Type::In || pinOutType != PinData::Type::Out) {
			ed::RejectNewItem(ImVec4{ 1.0f, 0.f, 0.f, 1.f }, 2.f);
		}
		// pins in & out result in the same node..
		else if (nodeInId == nodeOutId) {
			ed::RejectNewItem(ImVec4{ 1.0f, 0.f, 0.f, 1.f }, 2.f);
		} 
		// ed::AcceptNewItem() return true when user release mouse button.
		else if (ed::AcceptNewItem()) {
			// we link them..
			// Controller::Node& nodeIn = nodes.at(nodeInId);
			Controller::Node& nodeOut = nodes.at(nodeOutId);

			// we create a new transition, only if the current list of transitions dont have nodeIn..
			if (std::ranges::find_if(nodeOut.transitions, [&](auto const& transition) { return transition.nextNode == nodeInId; }) == nodeOut.transitions.end()) {
				Controller::Transition newTransition{
					nodeInId,
					{}
				};
				nodeOut.transitions.push_back(newTransition);
			}
		}
	}
	else {
		ed::RejectNewItem(ImVec4{ 1.0f, 0.f, 0.f, 1.f }, 2.f);
	}
}

void AnimatorController::handleDeletion(Controller& controller) {
	ed::LinkId deletedLinkId;

	std::unordered_map<ControllerNodeID, std::unordered_set<int>> transitionsToDelete;

	while (ed::QueryDeletedLink(&deletedLinkId)) {
		// If you agree that link can be deleted, accept deletion.
		if (!ed::AcceptDeletedItem()) {
			continue;
		}

		auto iterator = linksToTransition.find(static_cast<std::size_t>(deletedLinkId));
		
		if (iterator == linksToTransition.end()) {
			continue;
		}

		auto&& [_, transition] = *iterator;
		transitionsToDelete[transition.node].insert(transition.index);
	}

	// so, how do we safely delete different elements in a vector where the index and iterator could be invalidated?
	// the trick is, you don't ;) you simply make a copy of the existing vector consisting only of the valid elements.
	for (auto&& [nodeId, transitionIndicesToDelete] : transitionsToDelete) {
		std::vector<Controller::Transition> nonDeletedTransitions;

		// can't be invalid, right?
		auto&& node = controller.data.nodes.at(nodeId);

		for (int i = 0; i < node.transitions.size(); ++i) {
			// this index has been indicated to be deleted.
			if (transitionIndicesToDelete.contains(i)) {
				continue;
			}

			nonDeletedTransitions.push_back(node.transitions[i]);
		}

		node.transitions = std::move(nonDeletedTransitions);
	}

	ed::NodeId deletedNodeId;

	while (ed::QueryDeletedNode(&deletedNodeId)) {
		if (static_cast<std::size_t>(deletedNodeId) == static_cast<std::size_t>(ENTRY_NODE)) {
			ed::RejectDeletedItem();
			continue;
		}
		else if (!ed::AcceptDeletedItem()) {
			continue;
		}
		
		ControllerNodeID nodeId = static_cast<std::size_t>(deletedNodeId);

		// because nodes are in an unorderd_map, handling iterator validation is much simpler.
		auto iterator = controller.data.nodes.find(nodeId);
		
		if (iterator == controller.data.nodes.end()) {
			assert(false && "The node graph shouldn't have invalid node id to begin with.");
			continue;
		}

		controller.data.nodes.erase(iterator);
	}
}

void AnimatorController::createNewCondition(Controller& controller, Controller::Transition& transition){
	// retrieve all parameters from the controller, and we default select the first parameter as our condition..
	
	if (controller.data.parameters.empty()) {
		// default construct one random condition?
		transition.conditions.push_back({});
		return;
	}

	auto const& parameter = controller.data.parameters.front();
	transition.conditions.push_back({ parameter.name, parameter.value });
}

void AnimatorController::displayParameterComboBox(Controller& controller, Controller::Condition& condition) {
	if (ImGui::BeginCombo("##Combo", condition.name.c_str())) {
		for (auto&& parameter : controller.data.parameters) {
			if (ImGui::Selectable(parameter.name.c_str(), parameter.name == condition.name)) {
				condition.name = parameter.name;
				condition.value = parameter.value;
			}
		}

		ImGui::EndCombo();
	}
}

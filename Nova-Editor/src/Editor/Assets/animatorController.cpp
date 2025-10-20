#include <utility>
#include <ranges>

#include "animatorController.h"

#include "imgui.h"
#include "imgui_node_editor.h"
#include "IconsFontAwesome6.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"

#include "component.h"
#include "controller.h"

#include "nova_math.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

namespace ed = ax::NodeEditor;

AnimatorController::AnimatorController(Editor& editor) :
	editor			{ editor },
	resourceManager	{ editor.resourceManager },
	assetManager	{ editor.assetManager },
	context			{ nullptr }
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

	displayLeftPanel(*controller);
	ImGui::SameLine();
	displayRightPanel(*animator, *controller);

	handleDragAndDrop(*controller);
	ImGui::End();
}

void AnimatorController::displayLeftPanel(Controller& controller) {
	ImGui::BeginChild("Left Panel", ImVec2(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
	
	if (ImGui::BeginTabBar("TabBar")) {
		if (ImGui::BeginTabItem("Parameter")) {
			displayParameterWindow(controller);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Selected Node")) {
			displaySelectedNodeProperties(controller);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::EndChild();
}

void AnimatorController::displayRightPanel([[maybe_unused]] Animator const& animator, Controller const& controller) {
	ImGui::BeginChild("Right Panel");

	ed::SetCurrentEditor(context);

	// Start of the node editor..
	ed::Begin("Node Editor", ImVec2(0.0f, 0.0f));

	int counter = 1;

	// Render all nodes..
	for (auto&& [id, node] : controller.data.nodes) {
		ImGui::PushID(static_cast<int>(static_cast<std::size_t>(id)));
		ed::NodeId nodeId = static_cast<std::size_t>(id);

		// Get animation name..
		auto* descriptor = assetManager.getDescriptor(node.animation);
		
		if (!descriptor) {
			continue;
		}

		// Render a node..
		ed::BeginNode(nodeId);
			ImGui::Text("%s", descriptor->name.c_str());

			ed::BeginPin(counter++, ed::PinKind::Input);
				ImGui::Text("-> In");
			ed::EndPin();

			ImGui::SameLine();

			ed::BeginPin(counter++, ed::PinKind::Output);
				ImGui::Text("Out ->");
			ed::EndPin();
		ed::EndNode();

		ImGui::PopID();
	}

	//
	// 2) Handle interactions
	//

	// Handle creation action, returns true if editor want to create new object (node or link)
	if (ed::BeginCreate()) {
		ed::PinId inputPinId, outputPinId;

		// QueryNewLink returns true if editor want to create new link between pins.
		if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
			if (inputPinId && outputPinId) {
				// We first verify
				
				// ed::AcceptNewItem() return true when user release mouse button.
				if (ed::AcceptNewItem()) {
				
				}
			}
		}
	}
	ed::EndCreate(); // Wraps up object creation action handling.

	ed::End();
	
	ImGui::EndChild();
}

void AnimatorController::displayParameterWindow(Controller& controller) {
	auto&& parameters = controller.data.parameters;

	ImVec2 child_size = ImGui::GetContentRegionAvail();
	child_size.y = 0.f;

	ImGui::BeginChild("Parameters Panel", child_size);

	if (editor.isInSimulationMode()) {
		ImGui::BeginDisabled();
	}

	static int counter = 0;

	// Display combo box to add parameter
	if (ImGui::BeginCombo("##", "[+] Add Paramter")) {
		std::string name = "Parameter ";
		if (ImGui::Selectable("Boolean")) {
			parameters.push_back({ name + std::to_string(counter++), false });
		}

		if (ImGui::Selectable("Integer")) {
			parameters.push_back({ name + std::to_string(counter++), 0 });
		}

		if (ImGui::Selectable("Float")) {
			parameters.push_back({ name + std::to_string(counter++), 0.f });
		}

		if (ImGui::Selectable("String")) {
			parameters.push_back({ name + std::to_string(counter++), "" });
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

		ImGui::TableHeadersRow(); 

		// Populate table rows
		int imguiCounter = 0;
		for (auto it = controller.data.parameters.begin(); it != controller.data.parameters.end();) {
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
				else if constexpr (std::same_as<std::string, ParameterType>) {
					typeInText = "string";
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
			std::visit([&](auto&& parameter) {
				using ParameterType = std::decay_t<decltype(parameter)>;

				// Column 3
				ImGui::TableSetColumnIndex(2);

				if constexpr (std::same_as<bool, ParameterType>) {
					ImGui::Checkbox("##value", &parameter);
				}
				else if constexpr (std::same_as<int, ParameterType>) {
					ImGui::InputInt("##value", &parameter, 0);
				}
				else if constexpr (std::same_as<float, ParameterType>) {
					ImGui::InputFloat("##value", &parameter);
				}
				else if constexpr (std::same_as<std::string, ParameterType>) {
					ImGui::InputText("##value", &parameter);
				}
				else {
					[&] <bool flag = false, typename T = ParameterType>() {
						static_assert(flag, "Not all types of variant accounted for." __FUNCSIG__);
					}();
				}
			}, variant);

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

	if (editor.isInSimulationMode()) {
		ImGui::EndDisabled();
	}

	ImGui::EndChild();
}

void AnimatorController::displaySelectedNodeProperties(Controller& controller) {
	ImVec2 child_size = ImGui::GetContentRegionAvail();
	child_size.y = 0.f;

	ImGui::BeginChild("Selected Node Properties", child_size);
	ImGui::EndChild();
}

void AnimatorController::handleDragAndDrop(Controller& controller) {
	if (!ImGui::BeginDragDropTarget()) {
		return;
	}

	ImGuiPayload const* payload = nullptr;
	if (payload = ImGui::AcceptDragDropPayload("DRAGGING_ANIMATION_ITEM"), !payload) {
		return;
	}

	auto&& [animationId, name] = *((std::pair<std::size_t, const char*>*)payload->Data);

	// only handle it if it's an animation.
	if (!editor.resourceManager.isResource<Model>(animationId)) {
		return;
	}

	ControllerNodeID nodeId = Math::getGUID();

	controller.data.nodes.insert({ nodeId, Controller::Node{
		nodeId,
		NO_CONTROLLER_NODE,
		NO_CONTROLLER_NODE,
		animationId
	}});
}

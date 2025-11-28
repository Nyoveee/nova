#include "Engine/engine.h"
#include "componentInspector.h"
#include "imgui.h"
#include "Editor/editor.h"
#include "component.h"
#include "ECS/ECS.h"
#include "AssetManager/assetManager.h"
#include "misc/cpp/imgui_stdlib.h"
#include "reflection.h"

#include "IconsFontAwesome6.h"

#include "displayComponent.h"

ComponentInspector::ComponentInspector(Editor& editor) :
	editor			{ editor },
	ecs				{ editor.engine.ecs },
	resourceManager { editor.engine.resourceManager },
	assetManager	{ editor.assetManager },
	audioSystem		{ editor.engine.audioSystem }
{}

void ComponentInspector::update() {
	ImGui::Begin(ICON_FA_WRENCH " Component Inspector");

	// Begin displaying entity meta data..
	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	// I code with the assumption of only 1 entity selected first.
	// @TODO: Extend for multi select.
	entt::entity selectedEntity = editor.getSelectedEntities()[0];
	entt::registry& registry = ecs.registry;
	
	// Display entity metadata.
	EntityData& entityData = registry.get<EntityData>(selectedEntity);
	if (ImGui::BeginTable("NameAndTagTable", 3, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Active Checkbox", ImGuiTableColumnFlags_WidthFixed, 70.f);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Tag", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();

		bool active = entityData.isActive;
		ImGui::Checkbox("Active?", &active);

		if (active != entityData.isActive) {
			ecs.setActive(selectedEntity, active);
		}

		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::InputText("Name", &entityData.name);

		ImGui::TableNextColumn();
		ImGui::InputText("Tag", &entityData.tag);

		ImGui::EndTable();
	}

	// Display a drop down list of all the available layers an entity can choose..
	auto& layers = editor.engine.ecs.sceneManager.layers;

	if (entityData.layerId < 0 || entityData.layerId >= layers.size()) {
		Logger::warn("Entity {} had invalid layer. Resetting it..", entityData.name);
		entityData.layerId = 0;
	}

	if (ImGui::BeginCombo("Render Layer", layers[entityData.layerId].name.c_str())) {
		for (int layerId = 0; layerId < layers.size(); ++layerId) {
			ImGui::PushID(layerId);

			auto& layer = layers[layerId];

			if (ImGui::Selectable(layer.name.c_str(), layerId == entityData.layerId)) {
				// Remove itself from the old layer..
				layers[entityData.layerId].entities.erase(selectedEntity);

				entityData.layerId = layerId;
				
				// Add itself into the new layer..
				layers[entityData.layerId].entities.insert(selectedEntity);
			}

			ImGui::PopID();
		}
		ImGui::EndCombo();
	}


	BasicAssetInfo* prefabAssetInfo = editor.assetManager.getDescriptor(entityData.prefabID);

	if (prefabAssetInfo) {
		ImGui::Separator();

		editor.displayAssetDropDownList<Prefab>(entityData.prefabID, "Prefab", nullptr);
		
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_BOX_OPEN)) {
			editor.unpackPrefab(entityData);
		}

		if (ImGui::Button("Update Prefab")) {
			editor.engine.prefabManager.updatePrefab(selectedEntity);
		}
	}
	else if (entityData.prefabID != TypedResourceID<Prefab>{ INVALID_RESOURCE_ID }) {
		Logger::warn("Entity {} has invalid prefab id, relegating him back to a normal entity..", entityData.name);
		editor.unpackPrefab(entityData);
	}

	ImGui::NewLine();

	if (ImGui::CollapsingHeader("Entity")) {
		ImGui::Text("Parent: ");
		ImGui::SameLine();
		ImGui::Text(entityData.parent == entt::null ? "None" : registry.get<EntityData>(entityData.parent).name.c_str());

		ImGui::Text("Direct children: ");

		ImGui::BeginChild("Direct children", ImVec2{ 0.f, 70.f }, ImGuiChildFlags_Borders);
		for (entt::entity child : entityData.children) {
			ImGui::BulletText(registry.get<EntityData>(child).name.c_str());
		}

		ImGui::EndChild();

		ImGui::Text("Prefab ID: %zu", static_cast<std::size_t>(entityData.prefabID));
	}

	// Display the rest of the components via reflection.
	//g_displayComponentFunctor(*this, selectedEntity, registry, true);
	g_displayComponentFunctor(editor, selectedEntity, registry, true);

	// Display add component button.
	displayComponentDropDownList<ALL_COMPONENTS>(selectedEntity);


	ImGui::End();
}

void ComponentInspector::displayAvailableScriptDropDownList(std::vector<ScriptData> const& ownedScripts, std::function<void(ResourceID)> onClickCallback)
{
	std::vector<ResourceID> const& allScripts{ resourceManager.getAllResources<ScriptAsset>() };
	
	if (ImGui::BeginCombo("Add new script", "##")) {
		// Add a search bar..
		ImGui::InputText("Search", &scriptSearchQuery);

		// Case insensitive search query..
		uppercaseScriptSearchQuery.clear();
		std::transform(scriptSearchQuery.begin(), scriptSearchQuery.end(), std::back_inserter(uppercaseScriptSearchQuery), [](char c) { return static_cast<char>(std::toupper(c)); });

		for (auto&& scriptID : allScripts) {
			auto compareID = [&](ScriptData const& ownedScript) { return scriptID == ownedScript.scriptId; };

			if (std::find_if(std::begin(ownedScripts), std::end(ownedScripts), compareID) != std::end(ownedScripts))
				continue;

			std::string const* namePtr = assetManager.getName(scriptID);

			if (!namePtr) {
				continue;
			}

			// Let's upper case our component name..
			uppercaseScriptName.clear();
			std::transform(namePtr->begin(), namePtr->end(), std::back_inserter(uppercaseScriptName), [](char c) { return static_cast<char>(std::toupper(c)); });

			if (uppercaseScriptName.find(uppercaseScriptSearchQuery) == std::string::npos) {
				continue;
			}

			ImGui::PushID(static_cast<int>(static_cast<std::size_t>(scriptID)));

			if (ImGui::Selectable(namePtr->c_str()))
				onClickCallback(scriptID);

			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
}

template<typename T>
void ComponentInspector::overrideProperties( T component, const char* dataMemberName) {
	entt::entity selectedEntity = editor.getSelectedEntities()[0];
	entt::registry& registry = ecs.registry;

	EntityData& entityData = registry.get<EntityData>(selectedEntity);
	int index{};
	bool sameName{ false };

	reflection::visit(
		[&](auto fieldData) {
			auto& dataMember = fieldData.get();
			constexpr const char* name = fieldData.name();

			if (name == dataMemberName) {
				sameName = true;
			}
			if (!sameName) {
				index++;
			}

		}, component);
	
	//std::cout << "\nTEST " << Family::id<T>() << std::endl;

	std::vector<int>::iterator it = std::find(entityData.overridenProperties[Family::id<T>()].begin(), entityData.overridenProperties[Family::id<T>()].end(), index);
	if (it == entityData.overridenProperties[Family::id<T>()].end()) {
		entityData.overridenProperties[Family::id<T>()].push_back(index);
	}

	std::sort(entityData.overridenProperties[Family::id<T>()].begin(), entityData.overridenProperties[Family::id<T>()].end());
	for (std::pair<std::size_t, std::vector<int>> pair : entityData.overridenProperties) {
		std::cout <<std::endl<< pair.first << " and ";
		for (int i : pair.second) {
			std::cout << i << " ";
		}
	}
}

#include "displayComponent.ipp"
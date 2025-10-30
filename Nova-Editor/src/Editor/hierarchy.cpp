#include "Engine/engine.h"
#include "editor.h"

#include "hierarchy.h"
#include "imgui.h"
#include "ECS/ECS.h"

#include "IconsFontAwesome6.h"
#include "component.h"
#include "Serialisation/serialisation.h"

#include <ranges>

Hierarchy::Hierarchy(Editor& editor) : 
	ecs		{ editor.engine.ecs },
	editor	{ editor }
{}

void Hierarchy::createGameObject(entt::registry& registry) {
	static int counter = 0;

	entt::entity entity = registry.create();
	std::cout << static_cast<int>(entity) << std::endl;
	registry.emplace<EntityData>(entity, EntityData{"Entity " + std::to_string(++counter) });
	registry.emplace<Transform>(entity);
}

void Hierarchy::displayEntityHierarchy(entt::entity entity, entt::registry& registry) {
	EntityData const& entityData = registry.get<EntityData>(entity);

	bool toDisplayTreeNode = false;

	ImGui::PushID(static_cast<unsigned>(entity));

	if (entityData.children.empty()) {
		ImGui::Bullet();
		ImGui::SameLine();

		if (ImGui::Selectable(entityData.name.c_str(), editor.isEntitySelected(entity))) {
			editor.selectEntities({ entity });
		}
	}
	else {
		// Display children recursively..
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

		if (editor.isEntitySelected(entity)) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		toDisplayTreeNode = ImGui::TreeNodeEx(entityData.name.c_str(), flags);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			editor.selectEntities({ entity });
		}
	}
		
	// I want my widgets to be draggable, providing the entity id as the payload.
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("HIERARCHY_ITEM", &entity, sizeof(entt::entity));

		// Draw tooltip-style preview while dragging
		ImGui::Text(entityData.name.c_str());

		ImGui::EndDragDropSource();
	}

	// I want all my widgets to be a valid drop target.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			ecs.setEntityParent(childEntity, entity, registry);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();
	
	// recursively displays tree hierarchy..
	if (toDisplayTreeNode) {
		for (entt::entity child : entityData.children) {
			displayEntityHierarchy(child, registry);
		}

		ImGui::TreePop();
	}
}

void Hierarchy::displayEntityWindow()
{
	entt::registry& registry = ecs.registry;

	if (ImGui::Button(ICON_FA_PLUG_CIRCLE_PLUS "  Create new entity")) {
		createGameObject(registry);
	}

	ImGui::BeginChild("Entities", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
	for (auto&& [entity, entityData] : registry.view<EntityData>().each()) {
		// Any child entities will be displayed by the parent entity in a hierarchy. 
		if (entityData.parent != entt::null) {
			continue;
		}

		displayEntityHierarchy(entity, registry);
	}
	ImGui::EndChild();

	// If the drag target is the child window itself, this means removing parent from entity.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			ecs.removeEntityParent(childEntity, registry);
		}

		ImGui::EndDragDropTarget();
	}
}

void Hierarchy::displayUIWindow()
{
	entt::registry& uiRegistry = ecs.uiRegistry;

	if (ImGui::Button(ICON_FA_PLUG_CIRCLE_PLUS "  Create new UI")) {
		createGameObject(uiRegistry);
	}

	ImGui::BeginChild("UI", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
	for (auto&& [entity, entityData] : uiRegistry.view<EntityData>().each()) {
		// Any child entities will be displayed by the parent entity in a hierarchy. 
		if (entityData.parent != entt::null) {
			continue;
		}

		displayEntityHierarchy(entity, uiRegistry);
	}
	ImGui::EndChild();
	// If the drag target is the child window itself, this means removing parent from entity.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			ecs.removeEntityParent(childEntity, uiRegistry);
		}

		ImGui::EndDragDropTarget();
	}
}

void Hierarchy::update() {
	entt::registry& registry = ecs.registry;
	entt::registry& uiRegistry = ecs.uiRegistry;

	// Show all game objects..
	ImGui::Begin(ICON_FA_LIST " Hierarchy");

	if (ecs.sceneManager.hasNoSceneSelected()) {
		ImGui::Text("No scene loaded.");
		ImGui::TextWrapped("Create a new scene by dragging a scene from the content browser to the viewport!");

		ImGui::End();
		return;
	}

	ImGui::Text("Scene loaded: Sample Scene");
	ImGui::Text("Entities: %zu", registry.view<EntityData>().size());
	ImGui::SameLine();
	ImGui::Text("UI: %zu", uiRegistry.view<EntityData>().size());

#if 0
	ImGui::Text("Selected entity: ");
	ImGui::SameLine();

	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("None.");
	}
	else {
		std::string text = "";

		for (entt::entity entity : editor.getSelectedEntities()) {
			// @TODO: figure out
			std::string const& name = registry.get<EntityData>(entity).name;
			text += name + " (id: " + std::to_string(static_cast<unsigned int>(entity)) + ") ";
		}

		ImGui::Text(text.c_str());
	}
#endif
	ImGui::Separator();


	if (ImGui::BeginTabBar("TabBar")) {
		if (ImGui::BeginTabItem("Entities")) {
			displayEntityWindow();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("UI")) {
			displayUIWindow();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}
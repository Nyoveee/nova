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

void Hierarchy::createGameObject() {
	entt::registry& registry = ecs.registry;
	static int counter = 0;

	entt::entity entity = registry.create();
	std::cout << static_cast<int>(entity) << std::endl;
	registry.emplace<EntityData>(entity, EntityData{"Entity " + std::to_string(++counter) });
	registry.emplace<Transform>(entity);
}

void Hierarchy::displayEntityHierarchy(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	EntityData const& entityData = registry.get<EntityData>(entity);

	bool toDisplayTreeNode = false;

	ImGui::PushID(static_cast<unsigned>(entity));
	auto hasHierarchyPrefab = [&entityData,&registry]() {
		EntityData root{ const_cast<EntityData&>(entityData) };
		while (root.parent != entt::null) {
			if (root.prefabID != INVALID_RESOURCE_ID)
				return true;
			root = registry.get<EntityData>(root.parent);
		
		}
		return root.prefabID != INVALID_RESOURCE_ID;
	};
	ImGui::PushStyleColor(ImGuiCol_Text, hasHierarchyPrefab() ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 1, 1));
	if (entityData.children.empty()) {
		ImGui::Indent(27.5f);
		if (ImGui::Selectable((ICON_FA_CUBE + std::string{ " " } + entityData.name).c_str(), editor.isEntitySelected(entity))) {
			editor.selectEntities({ entity });
		}

		// Check for double-click to focus on entity
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			Transform* transform = registry.try_get<Transform>(entity);
			if (transform) {
				editor.engine.cameraSystem.focusOnPosition(transform->position);
			}
		}
		ImGui::Unindent(27.5f);
	}
	else {
		// Display children recursively..
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

		if (editor.isEntitySelected(entity)) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		toDisplayTreeNode = ImGui::TreeNodeEx((ICON_FA_CUBE + std::string{ " " } + entityData.name).c_str(), flags);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			editor.selectEntities({ entity });
		}

		// Check for double-click to focus on entity
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			Transform* transform = registry.try_get<Transform>(entity);
			if (transform) {
				editor.engine.cameraSystem.focusOnPosition(transform->position);
			}
		}
	}
	ImGui::PopStyleColor();
	// I want my widgets to be draggable, providing the entity id as the payload.
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("HIERARCHY_ITEM", &entity, sizeof(entt::entity*));

		// Draw tooltip-style preview while dragging
		ImGui::Text(entityData.name.c_str());

		ImGui::EndDragDropSource();
	}

	// I want all my widgets to be a valid drop target.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			ecs.setEntityParent(childEntity, entity);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();

	// recursively displays tree hierarchy..
	if (toDisplayTreeNode) {
		for (entt::entity child : entityData.children) {
			displayEntityHierarchy(child);
		}

		ImGui::TreePop();
	}
}

void Hierarchy::update() {
	entt::registry& registry = ecs.registry;

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

	ImGui::Separator();

	if (ImGui::Button(ICON_FA_PLUG_CIRCLE_PLUS "  Create new entity")) {
		createGameObject();
	}

	ImGui::BeginChild("Entities", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);

	isHovering = ImGui::IsWindowHovered();

	for (auto&& [entity, entityData] : registry.view<EntityData>().each()) {
		// Any child entities will be displayed by the parent entity in a hierarchy. 
		if (entityData.parent != entt::null) {
			continue;
		}

		displayEntityHierarchy(entity);
	}

	ImGui::EndChild();

	// If the drag target is the child window itself, this means removing parent from entity.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			ecs.removeEntityParent(childEntity);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::End();
}
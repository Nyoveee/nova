#include "engine.h"
#include "componentInspector.h"
#include "imgui.h"
#include "editor.h"
#include "Component/component.h"
#include "ECS.h"

#include "misc/cpp/imgui_stdlib.h"

#include "IconsFontAwesome6.h"

namespace {
	// ================================================================================
	// I love C++ TEMPLATE META PROGRAMMING :D 
	// ================================================================================

	// displayComponent, like the function suggests, is responsible for displaying a component's properties in the inspector UI.
	template <typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component);

	// This functor allows the use of variadic template arguments to recursively invoke displayComponent, such
	// that it displays all the listed components of a given entity.
	template <typename T, typename... Components>
	void displayIndividualComponent(ComponentInspector& componentInspector, entt::entity entity) {
		entt::registry& registry = componentInspector.ecs.registry;

		if (registry.all_of<T>(entity)) {
			displayComponent<T>(componentInspector, entity, registry.get<T>(entity));
		}

		if constexpr (sizeof...(Components) > 0) {
			displayIndividualComponent<Components...>(componentInspector, entity);
		}
	}

	// Using functors such that we can "store" types into our objects. (woah!)
	template <typename... Components>
	struct ComponentFunctor {
		void operator()(ComponentInspector& componentInspector, entt::entity entity) const {
			displayIndividualComponent<Components...>(componentInspector, entity);
		}
	};

	// ================================================================================
	// List all the components you want to show in the component inspector UI here!!
	// Make sure you reflect the individual data members you want to show.
	// ================================================================================
	ComponentFunctor<
		Transform,
		MeshRenderer,
		Light
		// + add here
	> 
	g_displayComponentFunctor{};
}

ComponentInspector::ComponentInspector(Editor& editor) :
	editor			{ editor },
	ecs				{ editor.engine.ecs },
	assetManager	{ editor.engine.assetManager }
{}

void ComponentInspector::update() {
	imguiCounter = 0;

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
	
	ImGui::InputText("Name", &entityData.name);
	ImGui::NewLine();

	if (ImGui::CollapsingHeader("Entity")) {
		ImGui::Text("Parent: ");
		ImGui::SameLine();
		ImGui::Text(entityData.parent == entt::null ? "None" : registry.get<EntityData>(entityData.parent).name.c_str());

		ImGui::Text("Direct children: ");

		if (ImGui::BeginChild("Direct children", ImVec2{ 0.f, 70.f }, ImGuiChildFlags_Borders)) {
			for (entt::entity child : entityData.children) {
				ImGui::BulletText(registry.get<EntityData>(child).name.c_str());
			}

			ImGui::EndChild();
		}
	}

	// Display the rest of the components via reflection.
	g_displayComponentFunctor(*this, selectedEntity);

	ImGui::End();
}

#include "displayComponent.ipp"
#include "componentInspector.h"
#include "imgui.h"
#include "editor.h"
#include "Component/component.h"
#include "ECS.h"

#include "misc/cpp/imgui_stdlib.h"
#include <memory>

namespace {
	//#define ComponentsToDisplay \
	//	g_displayComponentFunctor = std::make_unique<>

	// welcome to my favourite template metaprogramming strategy trick of all.
	// is this an abomination? who knows.
	// please ask me for explanation if you need.
	struct Functor {
		virtual ~Functor() = 0 {};
		virtual void displayComponent(ComponentInspector& componentInspector, entt::entity entity) = 0;
	};

	// Eventually this function will be invoked 
	template <typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component);

	template <typename T, typename... Components>
	void displayIndividualComponent(ComponentInspector& componentInspector, entt::entity entity) {
		entt::registry& registry = componentInspector.ecs.registry;

		if (registry.all_of<T>(entity)
		) {
			::displayComponent<T>(componentInspector, entity, registry.get<T>(entity));
		}

		if constexpr (sizeof...(Components) > 0) {
			displayIndividualComponent<Components...>(componentInspector, entity);
		}
	}

	template <typename... Components>
	struct ComponentFunctor : public Functor {
		void displayComponent(ComponentInspector& componentInspector, entt::entity entity) final {
			displayIndividualComponent<Components...>(componentInspector, entity);
		}
	};

	std::unique_ptr<Functor> g_displayComponentFunctor = nullptr;
}

ComponentInspector::ComponentInspector(Editor& editor, ECS& ecs) :
	editor	{ editor },
	ecs		{ ecs }
{
	g_displayComponentFunctor = std::make_unique<
		ComponentFunctor<
			Transform,
			MeshRenderer
		>
	>();
}

void ComponentInspector::update() {
	ImGui::Begin("Component Inspector");

	// Begin displaying entity meta data..
	if (editor.selectedEntities.empty()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	// I code with the assumption of only 1 entity selected first.
	// @TODO: Extend for multi select.
	entt::entity selectedEntity = editor.selectedEntities[0];
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
	g_displayComponentFunctor->displayComponent(*this, selectedEntity);

	ImGui::End();
}

#include "displayComponent.ipp"
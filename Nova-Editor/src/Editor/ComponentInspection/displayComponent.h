#pragma once

namespace {
	// ================================================================================
	// I love C++ TEMPLATE META PROGRAMMING :D 
	// ================================================================================

	// displayComponent, like the function suggests, is responsible for displaying a component's properties in the inspector UI.
	template <typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component, bool b, entt::registry& registry);
	//void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component);

	// This functor allows the use of variadic template arguments to recursively invoke displayComponent, such
	// that it displays all the listed components of a given entity.
	template <typename T, typename... Components>
	void displayIndividualComponent(ComponentInspector& componentInspector, entt::entity entity, entt::registry& registry, bool b) {

		if (registry.all_of<T>(entity)) {
			displayComponent<T>(componentInspector, entity, registry.get<T>(entity), b, registry);
		}

		if constexpr (sizeof...(Components) > 0) {
			displayIndividualComponent<Components...>(componentInspector, entity, registry, b);
		}
	}

	// Using functors such that we can "store" types into our objects. (woah!)
	template <typename... Components>
	struct ComponentFunctor {
		void operator()(ComponentInspector& componentInspector, entt::entity entity, entt::registry& registry, bool b) const {
			displayIndividualComponent<Components...>(componentInspector, entity, registry, b);
		}
	};

	// ================================================================================
	// List all the components you want to show in the component inspector UI here!!
	// Make sure you reflect the individual data members you want to show.
	// ================================================================================
	ComponentFunctor<
		ALL_COMPONENTS
	>

	g_displayComponentFunctor{};
}
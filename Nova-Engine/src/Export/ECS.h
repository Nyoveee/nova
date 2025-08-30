#pragma once

#include "export.h"
#include <entt/entt.hpp>

class Engine;

// A ECS wrapper around the entt framework for ease of access from other classes.
class ECS {
public:
	DLL_API ECS(Engine& engine);

	DLL_API ~ECS();
	DLL_API ECS(ECS const& other)				= delete;
	DLL_API ECS(ECS&& other)					= delete;
	DLL_API ECS& operator=(ECS const& other)	= delete;
	DLL_API ECS& operator=(ECS&& other)			= delete;

public:
	// Set newParentEntity as the new parent for childEntity.
	// You can pass entt::null as the new parent and this makes the child entity a root entity with no parent.
	DLL_API void setEntityParent(entt::entity childEntity, entt::entity newParentEntity);
	DLL_API void removeEntityParent(entt::entity childEntity);

	// Finds out if a given entity is a descendant of parent (direct and indirect children).
	DLL_API bool isDescendantOf(entt::entity entity, entt::entity parent);

public:
	// This makes a copy of the registry. We need to indicate the components to copy.
	template <typename ...Components>
	void makeRegistryCopy();

	// This rolls back the registry to the previous copied state.
	template <typename ...Components>
	void rollbackRegistry();

public:
	// public!
	entt::registry registry;

private:
	Engine& engine;

	// We make a copy of the registry when the engine stars simulation mode.
	// We rollback to the original state when we end simulation mode.
	entt::registry copiedRegistry;
};

template <typename ...Components>
void ECS::makeRegistryCopy() {
	copiedRegistry.clear();

	// iterate through all the entities and make a copy.
	for (auto entity : registry.view<entt::entity>()) { 
		entity = copiedRegistry.create(entity);

		// We use fold expression to perform a certain action. 
		// Because a lamda is 1 expression, we can utilise the comma operator to invoke the lambda for every component.
		([&]() {
			auto* component = registry.try_get<Components>(entity);

			if (component) {
				copiedRegistry.emplace<Components>(entity, *component);
			}
		}(), ...);
	}
}

template <typename ...Components>
void ECS::rollbackRegistry() {
	registry.clear();

	// iterate through all the entities and make a copy.
	for (auto entity : copiedRegistry.view<entt::entity>()) {
		entity = registry.create(entity);

		// We use fold expression to perform a certain action. 
		// Because a lamda is 1 expression, we can utilise the comma operator to invoke the lambda for every component.
		([&]() {
			auto* component = copiedRegistry.try_get<Components>(entity);

			if (component) {
				registry.emplace<Components>(entity, std::move(*component));
			}
		}(), ...);
	}
}

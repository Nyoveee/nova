#pragma once

#include "export.h"
#include <entt/entt.hpp>

#include "component.h"
#include "SceneManager.h"

class Engine;

// A ECS wrapper around the entt framework for ease of access from other classes.
class ECS {
public:
	ENGINE_DLL_API ECS(Engine& engine);

	ENGINE_DLL_API ~ECS();
	ENGINE_DLL_API ECS(ECS const& other)				= delete;
	ENGINE_DLL_API ECS(ECS&& other)					= delete;
	ENGINE_DLL_API ECS& operator=(ECS const& other)	= delete;
	ENGINE_DLL_API ECS& operator=(ECS&& other)			= delete;

public:
	// Set newParentEntity as the new parent for childEntity.
	// You can pass entt::null as the new parent and this makes the child entity a root entity with no parent.
	ENGINE_DLL_API void setEntityParent(entt::entity childEntity, entt::entity newParentEntity, entt::registry& _registry);
	ENGINE_DLL_API void removeEntityParent(entt::entity childEntity, entt::registry& _registry);

	// Finds out if a given entity is a descendant of parent (direct and indirect children).
	ENGINE_DLL_API bool isDescendantOf(entt::entity entity, entt::entity parent, entt::registry& _registry);

	// this deletes an entity whilst preserving transform hirearchy invariant.
	ENGINE_DLL_API void deleteEntity(entt::entity entity, entt::registry& _registry);

public:
	// This makes a copy of the registry. We need to indicate the components to copy.
	template <typename ...Components>
	void makeRegistryCopy();

	// This rolls back the registry to the previous copied state.
	template <typename ...Components>
	void rollbackRegistry();

	// this copies the entities in a given vector
	template<typename ...Components>
	void copyVectorEntities(std::vector<entt::entity> const& entityVec);

	template<typename ...Components>
	void copyEntity(entt::entity en, entt::entity parent);

public:
	// public!
	entt::registry registry;
	entt::registry uiRegistry;
	entt::dispatcher systemEventDispatcher; //note we probably only need one just giga dump all events in here lol. 
	SceneManager sceneManager;

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

template<typename ...Components>
void ECS::copyVectorEntities(std::vector<entt::entity> const& entityVec) {
	for (auto en : entityVec) {
		copyEntity<ALL_COMPONENTS>(en, entt::null);
	}
	//for (auto en : entityVec) {
	//	auto tempEntity = registry.create();
	//	([&]() {
	//		Components* component = registry.try_get<Components>(en);
	//		if (component) {
	//			registry.emplace<Components>(tempEntity, *component);
	//		}
	//		}(), ...);

	//	EntityData* ed = registry.try_get<EntityData>(en);
	//	if (ed->parent != entt::null) {
	//		EntityData* parent = registry.try_get<EntityData>(ed->parent);
	//		parent->children.push_back(tempEntity);

	//	}
	//}
}

template<typename ...Components>
void ECS::copyEntity(entt::entity en, entt::entity parent) {
	//create new entity
	auto tempEntity = registry.create();
	([&]() {
		Components* component = registry.try_get<Components>(en);
		if (component) {
			registry.emplace<Components>(tempEntity, *component);
		}
		}(), ...);

	EntityData& entityData = registry.get<EntityData>(tempEntity);

	// if there is a parent, push temp entity into the parent and set the parent of the temp entity
	if (parent != entt::null) {
		EntityData* p = registry.try_get<EntityData>(parent);
		p->children.push_back(tempEntity);

		entityData.parent = parent;
	}

	//if temp entity is a parent, call the function recursively
	if (!entityData.children.empty()) {
		EntityData& parentED = registry.get<EntityData>(en);
		entityData.children.clear();
		for (entt::entity child : parentED.children) {
			copyEntity<ALL_COMPONENTS>(child, tempEntity);
		}
	}

	//if the selected entity is a child
	EntityData* ed = registry.try_get<EntityData>(en);
	if (ed->parent != entt::null && parent == entt::null) {
		EntityData& p = registry.get<EntityData>(ed->parent);
		p.children.push_back(tempEntity);
	}
}

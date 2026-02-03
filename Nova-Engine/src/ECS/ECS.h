#pragma once

#include "export.h"
#include <entt/entt.hpp>

#include "component.h"
#include "SceneManager.h"
#include "Engine/prefabManager.h"
#include "Serialisation/serialisation.h"

class Engine;
class PrefabManager;
// A ECS wrapper around the entt framework for ease of access from other classes.
class ECS {
public:
	ENGINE_DLL_API ECS(Engine& engine);

	ENGINE_DLL_API ~ECS();
	ENGINE_DLL_API ECS(ECS const& other)			= delete;
	ENGINE_DLL_API ECS(ECS&& other)					= delete;
	ENGINE_DLL_API ECS& operator=(ECS const& other)	= delete;
	ENGINE_DLL_API ECS& operator=(ECS&& other)		= delete;

public:
	// Set newParentEntity as the new parent for childEntity.
	// You can pass entt::null as the new parent and this makes the child entity a root entity with no parent.
	ENGINE_DLL_API void setEntityParent(entt::entity childEntity, entt::entity newParentEntity, bool recalculateLocalTransform = true);
	ENGINE_DLL_API void setEntityParent(entt::entity childEntity, entt::entity newParentEntity, bool recalculateLocalTransform, entt::registry& p_registry);

	ENGINE_DLL_API void removeEntityParent(entt::entity childEntity);

	// Finds out if a given entity is a descendant of parent (direct and indirect children).
	ENGINE_DLL_API bool isDescendantOf(entt::entity entity, entt::entity parent);

	// this deletes an entity whilst preserving transform hierarchy invariant.
	ENGINE_DLL_API void deleteEntity(entt::entity entity);
	ENGINE_DLL_API void deleteEntity(entt::entity entity, entt::registry& registry);

	// this recursively disables or enables an entity hierarchy..
	ENGINE_DLL_API void setActive(entt::entity entity, bool isActive);

	ENGINE_DLL_API bool isParentCanvas(entt::entity entity);

	ENGINE_DLL_API void recordOriginalScene();
	ENGINE_DLL_API void restoreOriginalScene();

public:
	// This makes a copy of the registry. We need to indicate the components to copy.
	template <typename ...Components>
	void makeRegistryCopy();

	// This rolls back the registry to the previous copied state.
	template <typename ...Components>
	void rollbackRegistry();

	// this copies the entities in a given vector
	ENGINE_DLL_API void copyVectorEntities(std::vector<entt::entity> const& entityVec);

	ENGINE_DLL_API entt::entity copyEntity(entt::entity en);

#if false
	ENGINE_DLL_API entt::entity getEntityId(EntityGUID entityGuid);
	ENGINE_DLL_API void clearEntityGuidMapping();
#endif

	template<typename ...Components>
	entt::entity copyEntityRecursively(entt::entity en, std::unordered_map<entt::entity, entt::entity>& map);

	template<typename Component>
	bool isComponentActive(entt::entity entity);
	ENGINE_DLL_API bool isComponentActive(entt::entity entity, ComponentID componentID);

	template<typename Component>
	void setComponentActive(entt::entity entity, bool isActive);
	ENGINE_DLL_API void setComponentActive(entt::entity entity, ComponentID componentID, bool isActive);

private:
	ENGINE_DLL_API void deleteEntityRecursively(entt::entity entity);

public:
	// public!
	entt::registry registry;
	entt::dispatcher systemEventDispatcher; //note we probably only need one just giga dump all events in here lol. 

	SceneManager sceneManager;
	ResourceID originalScene;	// we store the original scene when starting simulation..


private:
	Engine& engine;

	// We make a copy of the registry when the engine stars simulation mode.
	// We rollback to the original state when we end simulation mode.
	entt::registry copiedRegistry;

	// we maintain mapping from entity guid to loaded entity..
	std::unordered_map<EntityGUID, entt::entity> entityGuidToEntityId;
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

	originalScene = sceneManager.getCurrentScene();
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

	sceneManager.currentScene = originalScene;
}

template<typename ...Components>
entt::entity ECS::copyEntityRecursively(entt::entity en, std::unordered_map<entt::entity, entt::entity>& map) {
	entt::id_type highestID = Serialiser::findLargestEntity(registry);
	auto tempEntity = registry.create(static_cast<entt::entity>(highestID + 1));

	map[en] = tempEntity;

	([&]() {
		Components* component = registry.try_get<Components>(en);
		if (component) {
			registry.emplace<Components>(tempEntity, *component);
		}
	}(), ...);

	std::vector<entt::entity> childVec;

	EntityData* entityData = registry.try_get<EntityData>(tempEntity);

	for (entt::entity child : entityData->children) {
		entt::entity childEntity = copyEntityRecursively<ALL_COMPONENTS>(child, map);

		EntityData* childData = registry.try_get<EntityData>(childEntity);
		childData->parent = tempEntity;

		childVec.push_back(childEntity);
	}

	entityData->children = std::move(childVec);
	return tempEntity;
}
// Eseentially a proxy function since engine.h can't be included in this file
template<typename Component>
bool ECS::isComponentActive(entt::entity entity) { return isComponentActive(entity, typeid(Component).hash_code()); }
template<typename Component>
void ECS::setComponentActive(entt::entity entity, bool isActive) { setComponentActive(entity, typeid(Component).hash_code(), isActive);}
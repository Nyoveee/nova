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
	ENGINE_DLL_API ECS(ECS const& other)				= delete;
	ENGINE_DLL_API ECS(ECS&& other)					= delete;
	ENGINE_DLL_API ECS& operator=(ECS const& other)	= delete;
	ENGINE_DLL_API ECS& operator=(ECS&& other)			= delete;

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
	template<typename ...Components>
	void copyVectorEntities(std::vector<entt::entity> const& entityVec);

	template<typename ...Components>
	//void copyEntity(entt::entity en);
	entt::entity copyEntity(entt::entity en);

	template<typename Component>
	bool isComponentActive(entt::entity entity);
	ENGINE_DLL_API bool isComponentActive(entt::entity entity, ComponentID componentID);

	template<typename Component>
	void setComponentActive(entt::entity entity, bool isActive);
	ENGINE_DLL_API void setComponentActive(entt::entity entity, ComponentID componentID, bool isActive);

private:
	ENGINE_DLL_API void deleteEntityRecursively(entt::entity entity);

#if false
	ENGINE_DLL_API void onCanvasCreation(entt::registry&, entt::entity entityID);
	ENGINE_DLL_API void onCanvasDestruction(entt::registry&, entt::entity entityID);
#endif

public:
	// public!
	entt::registry registry;
	entt::dispatcher systemEventDispatcher; //note we probably only need one just giga dump all events in here lol. 

	entt::entity canvasUi;
	SceneManager sceneManager;
	ResourceID originalScene;	// we store the original scene when starting simulation..

private:
	Engine& engine;

	// We make a copy of the registry when the engine stars simulation mode.
	// We rollback to the original state when we end simulation mode.
	entt::registry copiedRegistry;
	PrefabManager prefabManager;

	std::unordered_map<entt::entity, entt::entity> map;
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
void ECS::copyVectorEntities(std::vector<entt::entity> const& entityVec) {
	for (auto en : entityVec) {
		entt::entity rootEntity = copyEntity<ALL_COMPONENTS>(en);
		prefabManager.mapSerializedField(rootEntity, map);
		map.clear();
	}
}

template<typename ...Components>
//void ECS::copyEntity(entt::entity en) {
entt::entity ECS::copyEntity(entt::entity en) {
	//create new entity
	entt::id_type highestID = Serialiser::findLargestEntity(registry);
	auto tempEntity = registry.create(static_cast<entt::entity>(highestID + 1));

	([&]() {
		Components* component = registry.try_get<Components>(en);
		if (component) {
			registry.emplace<Components>(tempEntity, *component);
		}
		}(), ...);

	map[en] = tempEntity;

	std::vector<entt::entity> childVec;

	EntityData* entityData = registry.try_get<EntityData>(tempEntity);
	if (entityData->children.size()) {
		for ([[maybe_unused]] entt::entity child : entityData->children) {
			copyEntity<ALL_COMPONENTS>(child);

			childVec.push_back(map[child]);
		}

		entityData->children = childVec;

		//for each child, assign the parent to the current ecsEntity
		for (entt::entity childEn : childVec) {
			EntityData* childData = registry.try_get<EntityData>(childEn);
			childData->parent = tempEntity;
		}
	}
	return tempEntity;

#if 0
	auto tempEntity = registry.create();
	([&]() {
		Components* component = registry.try_get<Components>(en);
		if (component) {
			registry.emplace<Components>(tempEntity, *component);
		}
	}(), ...);

	map[en] = tempEntity;

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
#endif
}
// Eseentially a proxy function since engine.h can't be included in this file
template<typename Component>
bool ECS::isComponentActive(entt::entity entity) { return isComponentActive(entity, typeid(Component).hash_code()); }
template<typename Component>
void ECS::setComponentActive(entt::entity entity, bool isActive) { setComponentActive(entity, typeid(Component).hash_code(), isActive);}
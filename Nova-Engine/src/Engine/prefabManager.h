#pragma once

#include <entt/entt.hpp>
#include "resource.h"
#include "Serialisation/serialisation.h"

class ECS;
class Engine;
class ResourceManager;

class PrefabManager {
public:
	PrefabManager(Engine& engine);

	template<typename ...Components>
	void instantiatePrefab(ResourceID id);
	
private:

	template<typename ...Components>
	void instantiatePrefabRecursive(entt::entity prefabEntity);

private:
	entt::registry prefabRegistry;
	std::unordered_map<ResourceID, entt::entity> prefabMap;
	ResourceManager& resourceManager;
	entt::registry& ecsRegistry;
	ECS& ecs;
	
	//maps a prefab entity to a ecs entity
	std::unordered_map<entt::entity, entt::entity> map;
};

#include "ResourceManager/resourceManager.h"
#include "ECS/ECS.h"

template<typename ...Components>
void PrefabManager::instantiatePrefab(ResourceID id) {
	auto&& [resource, result] = resourceManager.getResource<Prefab>(id);

	std::vector<entt::entity> entityVec;

	const char* fileName = resource->getFilePath().string.c_str();

	//if id is not present in prefabMap, call deserialise function to update the prefabRegistry
	entt::entity entity;
	if (prefabMap.find(id) == prefabMap.end()) {
		entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry);
		prefabMap[id] = entity;
	}
	else {
		entity = prefabMap[id];
	}
	
	instantiatePrefabRecursive<ALL_COMPONENTS>(entity);

	map.clear();


	//if ID is not found in the map, deserialise it from file
	//if (prefabMap.find(id) == prefabMap.end()) {
		//entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id));
		//Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry, entityVec);

		/*if (entityVec.size() == 0) {
			return;
		}

		for (int i{}; i < entityVec.size(); i++) {
			entt::entity prefabEntity = prefabRegistry.create(entityVec[i]);
			([&]() {
				auto* component = ecsRegistry.try_get<Components>(entityVec[i]);

				if (component) {
					prefabRegistry.emplace<Components>(prefabEntity, *component);
				}
				}(), ...);

			if (i == entityVec.size() - 1) {
				prefabMap[id] = prefabEntity;
			}
		}*/

	//}

	//else {

	//	//store the children into a vector for the recursive function
	//	entt::entity entity = prefabMap[id];
	//	EntityData* entityData = prefabRegistry.try_get<EntityData>(entity);
	//	if (entityData->children.size()) {
	//		for (entt::entity child : entityData->children) {
	//			entityVec.push_back(child);
	//		}
	//	}
	
		

		//EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);

		//if (entityData->children.size()) {
		//	auto&& [entity] 
		//}

	//}

	//else {
	//	std::vector<entt::entity> entityVector;
	//	instantiatePrefabRecursive<ALL_COMPONENTS>(id, ecsRegistry, entityVector);

	//}
}

template<typename ...Components>
void PrefabManager::instantiatePrefabRecursive(entt::entity prefabEntity) {
//void PrefabManager::instantiatePrefabRecursive(ResourceID id, entt::registry& ecsRegistry, std::vector<entt::entity>& entityVec, int index) {

	entt::id_type highestID = Serialiser::findLargestEntity(ecsRegistry);
	entt::entity ecsEntity = ecsRegistry.create(static_cast<entt::entity>(highestID + 1));

	([&]() {
	auto* component = prefabRegistry.try_get<Components>(prefabEntity);

	if (component) {
		ecsRegistry.emplace<Components>(ecsEntity, *component);
	}
	}(), ...);

	//maps the newly created ecsEntity with the prefabEntity, this is use to identify the ecsEntity child after the recursive call
	map[prefabEntity] = ecsEntity;

	std::vector<entt::entity> childVec;

	//check for child, if there child, call the function recurisively for each child
	EntityData* entityData = ecsRegistry.try_get<EntityData>(ecsEntity);
	if (entityData->children.size()) {
		for ([[maybe_unused]] entt::entity child : entityData->children) {
			instantiatePrefabRecursive<ALL_COMPONENTS>(child);

			childVec.push_back(map[child]);
		}

		entityData->children = childVec;

		//for each child, assign the parent to the current ecsEntity
		for (entt::entity en : childVec) {
			EntityData* childData = ecsRegistry.try_get<EntityData>(en);
			childData->parent = ecsEntity;
		}
	}

	




	//if (index >= entityVec.size()) {
	//	return;
	//}

	//entt::entity entity;
	//if (prefabMap.find(id) == prefabMap.end()) {
	//	entity = prefabMap[id];
	//}
	//else {
	//	entity = entityVec[index];
	//}
	//
	//entt::id_type highestID = Serialiser::findLargestEntity(ecsRegistry);

	//entt::entity ecsEntity = ecsRegistry.create(static_cast<entt::entity>(highestID + 1));
	//([&]() {
	//	auto* component = prefabRegistry.try_get<Components>(entity);

	//	if (component) {
	//		ecsRegistry.emplace<Components>(ecsEntity, *component);
	//	}
	//}(), ...);


	//EntityData* entityData = ecsRegistry.try_get<EntityData>(ecsEntity);
	//if (entityData->children.size()) {
	//	for (entt::entity child : entityData->children) {
	//		instantiatePrefabRecursive(id, ecsRegistry, entityVec, index+1);
	//	}
	//}
	


}
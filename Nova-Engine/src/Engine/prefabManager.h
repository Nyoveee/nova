#pragma once

#include <entt/entt.hpp>
#include "resource.h"
#include "export.h"
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
	void instantiatePrefabRecursive(ResourceID id, entt::registry& ecsRegistry, std::vector<entt::entity>& entityVec);

	template<typename ...Components>
	//ENGINE_DLL_API  void testFunc();
	  void testFunc();


private:
	entt::registry prefabRegistry;
	std::unordered_map<ResourceID, entt::entity> prefabMap;
	ResourceManager& resourceManager;
	entt::registry& ecsRegistry;	
};

#include "ResourceManager/resourceManager.h"

template<typename ...Components>
void PrefabManager::instantiatePrefab(ResourceID id) {
	auto&& [resource, result] = resourceManager.getResource<Prefab>(id);

	std::vector<entt::entity> entityVec;

	const char* fileName = resource->getFilePath().string.c_str();

	//if ID is not found in the map, deserialise it from file
	if (prefabMap.find(id) == prefabMap.end()) {
		//entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry, prefabMap);
		Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry, entityVec);

		if (entityVec.size() == 0) {
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
		}

		//entt::entity prefabEntity = prefabRegistry.create(entity);
		//([&]() {
		//	auto* component = ecsRegistry.try_get<Components>(entity);

		//	if (component) {
		//		prefabRegistry.emplace<Components>(prefabEntity, *component);
		//	}
		//	}(), ...);

		

		//EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);

		//if (entityData->children.size()) {
		//	auto&& [entity] 
		//}

	}

	else {
		std::vector<entt::entity> entityVector;
		instantiatePrefabRecursive<ALL_COMPONENTS>(id, ecsRegistry, entityVector);

	}
}

template<typename ...Components>
void PrefabManager::instantiatePrefabRecursive(ResourceID id, entt::registry& ecsRegistry, std::vector<entt::entity>& entityVec) {

	entt::entity entity = prefabMap[id];
	entt::id_type highestID = Serialiser::findLargestEntity(ecsRegistry);

	entt::entity ecsEntity = ecsRegistry.create(static_cast<entt::entity>(highestID + 1));
	([&]() {
		auto* component = prefabRegistry.try_get<Components>(entity);

		if (component) {
			ecsRegistry.emplace<Components>(ecsEntity, *component);
		}
	}(), ...);


	EntityData* entityData = ecsRegistry.try_get<EntityData>(ecsEntity);
	if (entityData->children.size()) {
		for ([[maybe_unused]] entt::entity child : entityData->children) {
			instantiatePrefabRecursive(id, ecsRegistry, entityVec);
		}
	}
	


}
#include "ResourceManager/resourceManager.h"
#include "ECS/ECS.h"

template<typename ...Components>
entt::entity PrefabManager::instantiatePrefab(ResourceID id) {
	if constexpr (sizeof...(Components) == 0) {
		[] <bool flag = true>() {
			static_assert(flag, "You passed empty component list haha");
		}();
	}


	std::vector<entt::entity> entityVec;


	//if id is not present in prefabMap, call deserialise function to update the prefabRegistry
	entt::entity entity;
	if (prefabMap.find(id) == prefabMap.end()) {
		entity = loadPrefab(id);

		if (entity == entt::null) {
			return entt::null;
		}
	}
	else {
		entity = prefabMap[id];
	}


	entt::entity newPrefabInstanceId = instantiatePrefabRecursive<ALL_COMPONENTS>(entity);

	mapSerializedField(newPrefabInstanceId, map);

	map.clear();


#if 0
	// if ID is not found in the map, deserialise it from file
	if (prefabMap.find(id) == prefabMap.end()) {
		entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id));
		Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry, entityVec);

		if (entityVec.size() == 0) {
			return entt::null;
		}
#endif

	//for (int i{}; i < entityVec.size(); i++) {
	//	entt::entity prefabEntity = prefabRegistry.create(entityVec[i]);

	//	([&]() {
	//		auto* component = ecsRegistry.try_get<Components>(entityVec[i]);
	//		if (component) {
	//			prefabRegistry.emplace<Components>(prefabEntity, *component);
	//		}
	//		}(), ...);

	//	if (i == entityVec.size() - 1) {
	//		prefabMap[id] = prefabEntity;
	//	}
	//}

#if 0
	entt::entity prefabEntity = prefabRegistry.create(entity);
	([&]() {
		auto* component = ecsRegistry.try_get<Components>(entity);

		if (component) {
			prefabRegistry.emplace<Components>(prefabEntity, *component);
		}
		}(), ...);



	EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);

	if (entityData->children.size()) {
		auto&& [entity] 
	}
#endif

	return newPrefabInstanceId;
}

template<typename ...Components>
entt::entity PrefabManager::instantiatePrefabRecursive(entt::entity prefabEntity) {
	//entt::entity PrefabManager::instantiatePrefabRecursive(ResourceID id, entt::registry& ecsRegistry, [[maybe_unused]] std::vector<entt::entity>& entityVec, int index) {

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

	// asdsa

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

#if 0
	if (index >= entityVec.size()) {
		return;
	}

	entt::entity entity;
	if (prefabMap.find(id) == prefabMap.end()) {
		entity = prefabMap[id];
	}
	else {
		entity = entityVec[index];
	}

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
		for (entt::entity child : entityData->children) {
			instantiatePrefabRecursive(id, ecsRegistry, entityVec, index + 1);
		}
	}
#endif

	return ecsEntity;
}
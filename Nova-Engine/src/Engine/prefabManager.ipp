#include "ResourceManager/resourceManager.h"
#include "ECS/ECS.h"

template<typename ...Components>
entt::entity PrefabManager::instantiatePrefabRecursive(PrefabEntityID prefabEntity) {
	entt::id_type highestID = Serialiser::findLargestEntity(ecsRegistry);
	entt::entity ecsEntity = ecsRegistry.create(static_cast<entt::entity>(highestID + 1));

	([&]() {
		auto* component = prefabRegistry.try_get<Components>(prefabEntity);

		if (component) {
			ecsRegistry.emplace<Components>(ecsEntity, *component);
		}
	}(), ...);

	// maps the newly created ecsEntity with the prefabEntity, this is use to identify the ecsEntity child after the recursive call
	prefabEntityIdToInstanceId[prefabEntity] = ecsEntity;

	EntityData* entityData = ecsRegistry.try_get<EntityData>(ecsEntity);

	if (!entityData) {
		Logger::error("Instantiation of prefab failed. Prefab entity {} is invalid.", static_cast<unsigned>(prefabEntity));
		return entt::null;
	}

	entityData->prefabMetaData.prefabEntity = prefabEntity;

	// Contains the mapped entity id for children of the prefab instance.
	std::vector<entt::entity> childrenEntityIds;
	childrenEntityIds.reserve(entityData->children.size());

	// Recursively instantiate children of this prefab..
	
	// id of children is still in prefab entity domain..
	for (PrefabEntityID child : entityData->children) {
		entt::entity childPrefabInstanceId = instantiatePrefabRecursive<ALL_COMPONENTS>(child);

		EntityData* childEntityData = ecsRegistry.try_get<EntityData>(childPrefabInstanceId);

		if (!childEntityData) {
			Logger::error("Failed to instantiate {}.", static_cast<unsigned>(child));			
			continue;
		}

		childEntityData->parent = ecsEntity;
		childrenEntityIds.push_back(childPrefabInstanceId);
	}

	entityData->children = std::move(childrenEntityIds);

	return ecsEntity;
}
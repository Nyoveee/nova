#include "prefabManager.h"
#include "Serialisation/serialisation.h"
#include "component.h"
#include "engine.h"

PrefabManager::PrefabManager(Engine& engine) :
	resourceManager { engine.resourceManager },
	ecsRegistry		{ engine.ecs.registry },
	ecs				{ engine.ecs }
{}

entt::registry& PrefabManager::getPrefabRegistry() {
	return prefabRegistry;
}

std::unordered_map<ResourceID, entt::entity> PrefabManager::getPrefabMap() {
	return prefabMap;
}

entt::entity PrefabManager::loadPrefab(ResourceID id) {
	auto&& [resource, result] = resourceManager.getResource<Prefab>(id);

	if (!resource) {
		return entt::null;
	}

	const char* fileName = resource->getFilePath().string.c_str();

	entt::entity entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry);
	prefabMap[id] = entity;

	return entity;
}

void PrefabManager::mapSerializedField(entt::entity entity, std::unordered_map<entt::entity, entt::entity> entityMap) {

	auto* scripts = ecsRegistry.try_get<Scripts>(entity);
	if (scripts != nullptr) {
		for (ScriptData& scriptDatas : scripts->scriptDatas) {
			for (FieldData& fields : scriptDatas.fields) {
				std::visit([&](auto&& value) {
					using Type = std::decay_t<decltype(value)>;

					if constexpr (std::same_as<Type, entt::entity>) {
						auto iterator = entityMap.find(value);
						if (iterator != entityMap.end()) {
							if (value != iterator->second) {
								value = iterator->second;
							}
						}
					}

					}, fields.data);
			}
		}
	}

	EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);
	for (entt::entity child : entityData->children) {
		mapSerializedField(child, map);
	}

}

void PrefabManager::broadcast(entt::entity prefabEntity) {
	EntityData* prefabEntityData = prefabRegistry.try_get<EntityData>(prefabEntity);

	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* ecsEntityData = ecsRegistry.try_get<EntityData>(entity);

		if (ecsEntityData->prefabMetaData.prefabEntity == prefabEntity) {
			updateComponents<ALL_COMPONENTS>(ecsRegistry, prefabRegistry, entity, prefabEntity);
		}
	}

	for (entt::entity child : prefabEntityData->children) {
		broadcast(child);
	}

#if 0
	entt::registry& ecsRegistry = ecs.registry;
	entt::registry& prefabRegistry = getPrefabRegistry();
	std::unordered_map<ResourceID, entt::entity> prefabMap = getPrefabMap();
	entt::entity prefabEntity = prefabMap[selectedResourceId];

	// find entities with the same prefabID
	for (entt::entity en : ecsRegistry.view<entt::entity>()) {
		EntityData* entityData = ecsRegistry.try_get<EntityData>(en);
		EntityData* prefabData = prefabRegistry.try_get<EntityData>(prefabEntity);
		if ((entityData->prefabID == selectedResourceId) && (entityData->name == prefabData->name)) {
			updateComponents<ALL_COMPONENTS>(ecsRegistry, prefabRegistry, en, prefabEntity);
		}
	}
#endif

}

void PrefabManager::prefabBroadcast() {
	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);
		if (entityData->prefabMetaData.prefabID != INVALID_RESOURCE_ID) {

			entt::entity prefabEntity;

			//checks if prefab is loaded
			auto iterator = prefabMap.find(entityData->prefabMetaData.prefabID);
			if (iterator == prefabMap.end()) {
				prefabEntity = loadPrefab(entityData->prefabMetaData.prefabID);
				if (prefabEntity == entt::null) {
					entityData->prefabMetaData.prefabID = INVALID_RESOURCE_ID;
				}
				continue;
			}
			else {
				prefabEntity = iterator->second;
			}

			//if (prefabMap.find(entityData->prefabMetaData.prefabID) == prefabMap.end()) {
			//	prefabEntity = loadPrefab(entityData->prefabMetaData.prefabID);
			//	if (prefabEntity == entt::null) {
			//		entityData->prefabMetaData.prefabID = INVALID_RESOURCE_ID;
			//	}
			//}
			
			//only broadcast the root
			EntityData* prefabData = prefabRegistry.try_get<EntityData>(prefabEntity);
			if (prefabData->name == entityData->name) {
				broadcast(entityData->prefabMetaData.prefabEntity);
			}
			//if (prefabData) {
	
			//}

		}
	}
}


void PrefabManager::updatePrefab(entt::entity prefabInstance) {
	//get the root prefab
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);
	auto iterator = prefabMap.find(entityData->prefabMetaData.prefabID);
	if (iterator == prefabMap.end()) {
		return;
	}

	if (entityData->prefabMetaData.prefabID == INVALID_RESOURCE_ID) {
		return;
	}

	updateFromPrefabInstance(iterator->second);
}

void PrefabManager::updateFromPrefabInstance(entt::entity prefabInstance) {

	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);
	updateComponents<ALL_COMPONENTS>(prefabRegistry, ecsRegistry, entityData->prefabMetaData.prefabEntity, prefabInstance);

	for (entt::entity child : entityData->children) {
		updateFromPrefabInstance(child);
	}
}

template<typename ...Components>
void PrefabManager::updateComponents(entt::registry& toRegistry, entt::registry& fromRegistry, entt::entity toEntity, entt::entity fromEntity) {

	([&]() {
		if (!std::is_same<EntityData, Components>::value) {
			auto* component = fromRegistry.try_get<Components>(fromEntity);

			EntityData* toEntityData = toRegistry.try_get<EntityData>(toEntity);
			bool overrideCheck{ false };

			//check for override check box
			if (toEntityData->overridenComponents.find(Family::id<Components>()) != toEntityData->overridenComponents.end()) {
				if (!toEntityData->overridenComponents[Family::id<Components>()]) {
					overrideCheck = true;
				}
			}

			if (&toRegistry == &prefabRegistry) {
				overrideCheck = true;
			}

			if (component && overrideCheck) {
				auto* entityComponent = toRegistry.try_get<Components>(toEntity);
				*entityComponent = *component;
			}
		}
		}(), ...);
}
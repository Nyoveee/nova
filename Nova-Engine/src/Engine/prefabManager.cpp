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

std::unordered_map<ResourceID, entt::entity> const& PrefabManager::getPrefabMap() const {
	return prefabMap;
}

PrefabEntityID PrefabManager::loadPrefab(ResourceID id) {
	auto&& [resource, result] = resourceManager.getResource<Prefab>(id);

	if (!resource) {
		return entt::null;
	}

	const char* fileName = resource->getFilePath().string.c_str();

	PrefabEntityID prefabEntityId = Serialiser::deserialisePrefab(fileName, TypedResourceID<Prefab>{ id }, prefabRegistry);

	if (prefabEntityId != entt::null) {
		prefabMap[id] = prefabEntityId;
	}

	return prefabEntityId;
}

entt::entity PrefabManager::instantiatePrefab(ResourceID id) {
	// if id is not present in prefabMap, call deserialise function to update the prefabRegistry
	PrefabEntityID prefabEntityId = entt::null;

	if (prefabMap.find(id) == prefabMap.end()) {
		prefabEntityId = loadPrefab(id);

		if (prefabEntityId == entt::null) {
			return entt::null;
		}
	}
	else {
		prefabEntityId = prefabMap.at(id);
	}
	
	// Locating a prefab entity based of invalid resource id.
	if (prefabEntityId == entt::null) {
		Logger::error("Attempting to instantiate a prefab with potentially invalid resource id or corrupted prefab file.");
		return entt::null;
	}

	entt::entity newPrefabInstanceId = instantiatePrefabRecursive<ALL_COMPONENTS>(prefabEntityId);

	if (newPrefabInstanceId != entt::null) {
		// Strong guarantee.
		ecsRegistry.get<EntityData>(newPrefabInstanceId).parent = entt::null;
		mapSerializedField(newPrefabInstanceId, prefabEntityIdToInstanceId);
	}

	prefabEntityIdToInstanceId.clear();

	return newPrefabInstanceId;
}

void PrefabManager::mapSerializedField(entt::entity entity, std::unordered_map<PrefabEntityID, entt::entity> const& entityMapping) {
	Scripts* scripts = ecsRegistry.try_get<Scripts>(entity);

	if (scripts != nullptr) {
		for (ScriptData& scriptDatas : scripts->scriptDatas) {
			for (FieldData& fields : scriptDatas.fields) {
				std::visit([&](auto&& value) {
					using Type = std::decay_t<decltype(value)>;

					if constexpr (std::same_as<Type, entt::entity>) {
						auto iterator = entityMapping.find(value);

						if (iterator != entityMapping.end()) {
							value = iterator->second;
						}
					}

				}, fields.data);
			}
		}
	}

	EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);

	if (!entityData) {
		return;
	}

	for (entt::entity child : entityData->children) {
		mapSerializedField(child, entityMapping);
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

entt::entity PrefabManager::getParent(entt::entity prefabInstance) {
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);
	if (entityData->parent == entt::null) {
		return prefabInstance;
	}

	entt::entity parent = entityData->parent;
	while (entityData->parent != entt::null) {
		parent = entityData->parent;
		entityData = ecsRegistry.try_get<EntityData>(parent);
		
		
	}
	return parent;
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



	//updateFromPrefabInstance(iterator->second);
	updateFromPrefabInstance(getParent(prefabInstance));
}

void PrefabManager::convertToPrefab(entt::entity entity, ResourceID id) {
	EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);
	entityData->prefabMetaData.prefabID = TypedResourceID<Prefab>{ static_cast<std::size_t>(id) };
	entityData->prefabID = TypedResourceID<Prefab>{ static_cast<std::size_t>(id) };

	for (entt::entity child : entityData->children) {
		convertToPrefab(child, id);
	}

}

void PrefabManager::updateFromPrefabInstance(entt::entity prefabInstance) {

	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);

	if (!entityData) {
		return;
	}

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

			//If toRegistry is prefabRegistry or the toRegistry does not contain the component
			auto* entityComponent = toRegistry.try_get<Components>(toEntity);
			if (&toRegistry == &prefabRegistry || entityComponent == nullptr) {
				overrideCheck = true;
			}

			if (component && overrideCheck) {
				auto* newEntityComponent = toRegistry.try_get<Components>(toEntity);
				if (newEntityComponent == nullptr) {
					toRegistry.emplace_or_replace<Components>(toEntity, Components{});
				}
				newEntityComponent = toRegistry.try_get<Components>(toEntity);
				
				*newEntityComponent = *component;
			}
		}
		}(), ...);
}
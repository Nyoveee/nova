#include "prefabManager.h"
#include "Serialisation/serialisation.h"
#include "component.h"
#include "engine.h"

PrefabManager::PrefabManager(Engine& engine) :
	resourceManager { engine.resourceManager },
	ecsRegistry		{ engine.ecs.registry },
	ecs				{ engine.ecs },
	firstTimeLoad	{ false }
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

	std::unordered_map<PrefabFileEntityID, PrefabEntityID> mapping;
	PrefabEntityID prefabEntityId = Serialiser::deserialisePrefab(fileName, TypedResourceID<Prefab>{ id }, prefabRegistry, mapping);

	if (prefabEntityId != entt::null) {
		prefabMap[id] = prefabEntityId;
		mapSerializedField(prefabRegistry, prefabEntityId, mapping);
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

void PrefabManager::mapSerializedField(entt::entity entity, std::unordered_map<entt::entity, entt::entity> const& entityMapping) {
	mapSerializedField(ecs.registry, entity, entityMapping);
}

void PrefabManager::mapSerializedField(entt::registry& registry, entt::entity entity, std::unordered_map<entt::entity, entt::entity> const& entityMapping) {
	Scripts* scripts = registry.try_get<Scripts>(entity);

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

	Button* button = registry.try_get<Button>(entity);

	if (button) {
		auto iterator = entityMapping.find(button->reference.entity);
		if (iterator != entityMapping.end()) {
			button->reference.entity = iterator->second;
		}
	}

	EntityData* entityData = registry.try_get<EntityData>(entity);

	if (!entityData) {
		return;
	}

	for (entt::entity child : entityData->children) {
		mapSerializedField(registry, child, entityMapping);
	}
}

void PrefabManager::broadcast(entt::entity prefabEntity) {
	EntityData* prefabEntityData = prefabRegistry.try_get<EntityData>(prefabEntity);

	//find every prefab instance
	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* ecsEntityData = ecsRegistry.try_get<EntityData>(entity);
		//double check with name
		if ( (ecsEntityData->prefabMetaData.prefabID != INVALID_RESOURCE_ID) && (prefabEntityData->name == ecsEntityData->name)) {
			updateComponents<ALL_COMPONENTS>(ecsRegistry, prefabRegistry, entity, prefabEntity);
		}
	}
	for (entt::entity child : prefabEntityData->children) {
		broadcast(child);
	}

}

void PrefabManager::prefabBroadcast(ResourceID prefabID) {
	//check if prefab is loaded
	entt::entity prefabEntity = entt::null;
	auto iterator = prefabMap.find(prefabID);
	if (iterator == prefabMap.end()) {
		prefabEntity = loadPrefab(prefabID);
	}
	else {
		prefabEntity = iterator->second;
	}
	
	if (prefabEntity == entt::null) {
		return;
	}
	else {
		broadcast(getParent(prefabEntity, prefabRegistry));
	}

#if 0
	//Check if prefab is loaded
	entt::entity prefabEntity = entt::null;

	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* entityData = ecsRegistry.try_get<EntityData>(entity);
		//if (entityData->prefabMetaData.prefabID != INVALID_RESOURCE_ID) {
		if (entityData->prefabMetaData.prefabID == prefabID) {

			//checks if prefab is loaded
			auto iterator = prefabMap.find(entityData->prefabMetaData.prefabID);
			if (iterator == prefabMap.end()) {
				prefabEntity = loadPrefab(entityData->prefabMetaData.prefabID);
				if (prefabEntity == entt::null) {
					entityData->prefabMetaData.prefabID = INVALID_RESOURCE_ID;
				}
				
			}
			else {
				prefabEntity = iterator->second;
				
			}

			EntityData* prefabEntityData = prefabRegistry.try_get<EntityData>(prefabEntity);
			std::cout << prefabEntityData->name << std::endl;
			
			////only broadcast the root, broadcast function will check for child
			//if ((entityData->prefabMetaData.prefabID != INVALID_RESOURCE_ID) && (prefabEntityData->name == entityData->name)) {
			//	broadcast(getParent(prefabEntity, prefabRegistry));
			//}
			

			//EntityData* prefabData = prefabRegistry.try_get<EntityData>(prefabEntity);
			//if (prefabData->name == entityData->name) {
			//	//entityData->prefabMetaData.prefabEntity = prefabEntity;
			//	//entityData->prefabMetaData.prefabID = prefabID;

			//	broadcast(prefabEntity);
			//}
		}
	}
	////only broadcast the root, broadcast function will check for child
	if (prefabEntity != entt::null) {
		(getParent(prefabEntity, prefabRegistry));
	}
#endif
}

entt::entity PrefabManager::getParent(entt::entity prefabInstance, entt::registry& registry) {
	EntityData* entityData = registry.try_get<EntityData>(prefabInstance);
	if (entityData->parent == entt::null) {
		return prefabInstance;
	}
	entt::entity parent = entt::null;
	if (&registry == &prefabRegistry) {
		
		while (entityData->parent != entt::null) {
			parent = entityData->parent;
			entityData = ecsRegistry.try_get<EntityData>(parent);
		}
	}
	else {
		ResourceID id = entityData->prefabMetaData.prefabID;
		parent = prefabInstance;
		while (true) {
			EntityData* parentData = registry.try_get<EntityData>(entityData->parent);
			if (parentData->prefabID == INVALID_RESOURCE_ID) {
				break;
			}
			parent = entityData->parent;
			entityData = ecsRegistry.try_get<EntityData>(parent);
		}
	}
	return parent;
}


void PrefabManager::updatePrefab(entt::entity prefabInstance) {
	//load every prefab based on the Instance
	if (!firstTimeLoad) {
		for (entt::entity en : ecsRegistry.view<entt::entity>()) {
			EntityData* ed = ecsRegistry.try_get<EntityData>(en);
			if (ed->prefabMetaData.prefabID != INVALID_RESOURCE_ID) {
				auto it = prefabMap.find(ed->prefabMetaData.prefabID);
				if (it == prefabMap.end()) {
					loadPrefab(ed->prefabMetaData.prefabID);
				}
			}
		}
		firstTimeLoad = true;
	}

	//get the root prefab
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);
	auto iterator = prefabMap.find(entityData->prefabMetaData.prefabID);
	if (iterator == prefabMap.end()) {
		return;
	}

	if (entityData->prefabMetaData.prefabID == INVALID_RESOURCE_ID) {
		return;
	}

	updateFromPrefabInstance(getParent(prefabInstance, ecsRegistry));
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
	//check for the prefab with the same name in the registry
	entt::entity prefabEntity = entt::null;
	std::cout << prefabMap.size();
	for (auto prefab : prefabRegistry.view<entt::entity>()) {
		EntityData* prefabData = getPrefabRegistry().try_get<EntityData>(prefab);
		if (prefabData->name == entityData->name) {
			prefabEntity = prefab;
			break;
		}
	}

	updateComponents<ALL_COMPONENTS>(prefabRegistry, ecsRegistry, prefabEntity, prefabInstance);

	for (entt::entity child : entityData->children) {
		updateFromPrefabInstance(child);
	}
}

template<typename ...Components>
void PrefabManager::updateComponents(entt::registry& toRegistry, entt::registry& fromRegistry, entt::entity toEntity, entt::entity fromEntity) {

	([&]() {
		if (!(std::is_same<EntityData, Components>::value || std::is_same<Transform, Components>::value)) {
			auto* component = fromRegistry.try_get<Components>(fromEntity);
			bool overrideCheck{ false };

			//check for override check box
			if (&toRegistry != &prefabRegistry) {
				EntityData* toEntityData = toRegistry.try_get<EntityData>(toEntity);
				if (!toEntityData->overridenComponents[Family::id<Components>()]) {
					overrideCheck = true;
				}
			}
			else {
				overrideCheck = true;
			}

			if (component && overrideCheck) {
				toRegistry.emplace_or_replace<Components>(toEntity, *component);
			}
		}
		}(), ...);
}
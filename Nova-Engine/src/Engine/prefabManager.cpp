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

	std::unordered_map<PrefabFileEntityID, PrefabEntityID> mapping;
	PrefabEntityID prefabEntityId = Serialiser::deserialisePrefab(fileName, TypedResourceID<Prefab>{ id }, prefabRegistry, mapping, entityGUIDToPrefabEntity);

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

	if (!prefabEntityData) {
		return;
	}

	// find every prefab instance
	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* ecsEntityData = ecsRegistry.try_get<EntityData>(entity);
		
		if (!ecsEntityData) {
			continue;
		}

		// update if prefab instance found.. (we only want to update ancestor..)
		if ((prefabEntityData->entityGUID == ecsEntityData->entityGUID) && getParent(entity, ecsRegistry) == entity) {
			// Handle mapping between prefab id and entt::entity..
			std::unordered_map<PrefabEntityID, entt::entity> mapping;
			broadcastHierarchy(entity, mapping);
			mapSerializedField(entity, mapping);
		}
	}
}

void PrefabManager::broadcastHierarchy(entt::entity ecsEntity, std::unordered_map<PrefabEntityID, entt::entity>& mapping) {
	EntityData& ecsEntityData = ecsRegistry.get<EntityData>(ecsEntity);

	auto iterator = entityGUIDToPrefabEntity.find(ecsEntityData.entityGUID);

	if (iterator == entityGUIDToPrefabEntity.end()) {
		return;
	}

	entt::entity prefabEntity = iterator->second;
	mapping[prefabEntity] = ecsEntity;

	updateComponents<ALL_COMPONENTS>(ecsRegistry, prefabRegistry, ecsEntity, prefabEntity);

	for (entt::entity child : ecsEntityData.children) {
		broadcastHierarchy(child, mapping);
	}
}

void PrefabManager::prefabBroadcast(ResourceID prefabID) {
	// Check if prefab is loaded
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

	broadcast(getParent(prefabEntity, prefabRegistry));
}

entt::entity PrefabManager::getParent(entt::entity prefabInstance, entt::registry& registry) {
	EntityData* entityData = registry.try_get<EntityData>(prefabInstance);

	if (!entityData) {
		return entt::null;
	}

	if (entityData->parent == entt::null) {
		return prefabInstance;
	}

	entt::entity parent = entt::null;

	if (&registry == &prefabRegistry) {
		while (entityData && entityData->parent != entt::null) {
			parent = entityData->parent;
			entityData = registry.try_get<EntityData>(parent);
		}
	}
	else {
		ResourceID id = entityData->prefabID;
		parent = prefabInstance;

		while (entityData) {
			EntityData* parentData = registry.try_get<EntityData>(entityData->parent);
			if (!parentData || !resourceManager.isResource<Prefab>(parentData->prefabID)) {
				break;
			}

			parent = entityData->parent;
			entityData = ecsRegistry.try_get<EntityData>(parent);
		}
	}
	return parent;
}


void PrefabManager::updatePrefab(entt::entity prefabInstance) {
	// Check prefab id of prefab instance..
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);

	if (!entityData) {
		return;
	}

	if (entityData->prefabID == INVALID_RESOURCE_ID) {
		return;
	}

	// prefab not loaded..
	if (prefabMap.find(entityData->prefabID) == prefabMap.end()) {
		loadPrefab(entityData->prefabID);
	}

	updateFromPrefabInstance(getParent(prefabInstance, ecsRegistry), entityData->prefabID);
	
	auto guidIterator = entityGUIDToPrefabEntity.find(entityData->entityGUID);
	
	if (guidIterator != entityGUIDToPrefabEntity.end()) {
		broadcast(getParent(guidIterator->second, prefabRegistry));
	}
}

void PrefabManager::prefabOverride(entt::entity prefabInstance) {
	// We want to override the original prefab, so we have to first clear the whole prefab registry with this associated prefab id..
	// Check prefab id of prefab instance..
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);

	if (!entityData) {
		return;
	}

	if (entityData->prefabID == INVALID_RESOURCE_ID) {
		return;
	}

	// prefab not loaded..
	if (prefabMap.find(entityData->prefabID) == prefabMap.end()) {
		loadPrefab(entityData->prefabID);
	}

	auto iterator = prefabMap.find(entityData->prefabID);

	// loading of prefab failed..
	if (iterator == prefabMap.end()) {
		return;
	}

	PrefabEntityID prefabId = iterator->second;

	auto guidIterator = entityGUIDToPrefabEntity.find(entityData->entityGUID);

	if (guidIterator != entityGUIDToPrefabEntity.end()) {
		prefabId = guidIterator->second;
		entityGUIDToPrefabEntity.erase(guidIterator);
	}

	// we delete the prefab..
	deletePrefab(prefabId);

	prefabMap.erase(entityData->prefabID);

	// we repopulate prefab with new data..
	prefabId = prefabRegistry.create(prefabId);
	prefabMap.insert({ entityData->prefabID, prefabId });

	repopulatesPrefab(prefabInstance, prefabId);

	// update prefab to ensure mapping..
	updatePrefab(prefabInstance);
}

void PrefabManager::guidRemap(ResourceID  prefabId) {
#if 0
	// prefab not loaded..
	if (prefabMap.find(prefabId) == prefabMap.end()) {
		loadPrefab(prefabId);
	}

	auto iterator = prefabMap.find(prefabId);

	// loading of prefab failed..
	if (iterator == prefabMap.end()) {
		return;
	}

	// we retrieve the corresponding root prefab entity..
	PrefabEntityID rootPrefabEntityId = iterator->second;
	EntityData const& prefabEntityData = prefabRegistry.get<EntityData>(rootPrefabEntityId);

	// In the existing scene, we find all entiy with this prefab id..
	for (entt::entity entity : ecsRegistry.view<entt::entity>()) {
		EntityData* ecsEntityData = ecsRegistry.try_get<EntityData>(entity);

		if (!ecsEntityData) {
			continue;
		}

		// we only want to update ancestor.. we recurse down..
		if (getParent(entity, ecsRegistry) == entity) {
			// let's recurse downwards..
			for (int i = 0; i < prefabEntityData.children.size() && i < ecsEntityData->children.size(); ++i) {

			}
		}
	}
#endif
}

void PrefabManager::deletePrefab(PrefabEntityID prefabId) {
	EntityData const& prefabEntityData = prefabRegistry.get<EntityData>(prefabId);
	
	for (PrefabEntityID child : prefabEntityData.children) {
		deletePrefab(child);
	}

	entityGUIDToPrefabEntity.erase(prefabEntityData.entityGUID);
	prefabRegistry.destroy(prefabId);
}

void PrefabManager::repopulatesPrefab(entt::entity prefabInstance, PrefabEntityID prefabId) {
	EntityData& entityData = ecsRegistry.get<EntityData>(prefabInstance);

	updateComponents<ALL_COMPONENTS>(prefabRegistry, ecsRegistry, prefabId, prefabInstance);
	entityGUIDToPrefabEntity[entityData.entityGUID] = prefabId;

	// Make a copy of the entityData and Transform..
	EntityData& newEntityData = prefabRegistry.emplace<EntityData>(prefabId, entityData);
	prefabRegistry.emplace<Transform>(prefabId, ecsRegistry.get<Transform>(prefabInstance));

	// clear child vector from the new entity.. we will recursively update it..
	newEntityData.children.clear();

	// Let's check if the parent of this new entity's GUID is valid..
	if (entityData.parent != entt::null) {
		EntityData const& parentEntityData = ecsRegistry.get<EntityData>(entityData.parent);
		auto iterator = entityGUIDToPrefabEntity.find(parentEntityData.entityGUID);

		// Valid.. let's integrate this new entity to part of the prefab..
		if (iterator != entityGUIDToPrefabEntity.end()) {
			// update this new prefab entity's parent.. 
			newEntityData.parent = iterator->second;

			EntityData& parentPrefabData = prefabRegistry.get<EntityData>(iterator->second);
			parentPrefabData.children.push_back(prefabId);
		}
	}

	for (entt::entity child : entityData.children) {
		PrefabEntityID newPrefabId = prefabRegistry.create();
		repopulatesPrefab(child, newPrefabId);
	}
}

void PrefabManager::updateFromPrefabInstance(entt::entity prefabInstance, ResourceID prefabId) {
	EntityData* entityData = ecsRegistry.try_get<EntityData>(prefabInstance);
	
	if (!entityData) {
		return;
	}

	entt::entity prefabEntity = entt::null;

	// We locate the corresponding prefab id via the entity GUID. entity GUID is shared between ecs entity and prefab entity.
	auto iterator = entityGUIDToPrefabEntity.find(entityData->entityGUID);

	if (iterator == entityGUIDToPrefabEntity.end()) {
		if (entityData->parent != entt::null) {
			// No prefab entity found for corresponding entity.
			// This means that the entity we encounter is a new entity.. let's update the prefab to include this new entity..

			// Let's check if the parent of this new entity's GUID is valid..
			EntityData const& parentEntityData = ecsRegistry.get<EntityData>(entityData->parent);
			iterator = entityGUIDToPrefabEntity.find(parentEntityData.entityGUID);

			// Valid.. let's integrate this new entity to part of the prefab..
			if (iterator != entityGUIDToPrefabEntity.end()) {
				// Update its prefab metadata..
				entityData->prefabID = { prefabId };

				// Create the new prefab, and make a copy of the entityData and Transform..
				prefabEntity = prefabRegistry.create();
				EntityData& newEntityData = prefabRegistry.emplace<EntityData>(prefabEntity, *entityData);
				prefabRegistry.emplace<Transform>(prefabEntity, ecsRegistry.get<Transform>(prefabInstance));

				// clear child vector from the new entity.. we will recursively update it..
				newEntityData.children.clear();

				// update this new prefab entity's parent.. 
				newEntityData.parent = iterator->second;

				EntityData& parentPrefabData = prefabRegistry.get<EntityData>(iterator->second);
				parentPrefabData.children.push_back(prefabEntity);

				// remember to update guid database..
				entityGUIDToPrefabEntity.insert({ entityData->entityGUID, prefabEntity });
			}
		}
	}
	else {
		prefabEntity = iterator->second;
	}

	// We update the prefab 
	if (prefabEntity != entt::null) {
		// Clear all old components except for transform or entitydata..
		removeComponents<ALL_COMPONENTS>(prefabRegistry, prefabEntity);
		updateComponents<ALL_COMPONENTS>(prefabRegistry, ecsRegistry, prefabEntity, prefabInstance);

		// We have made a copy.. let's map serialized fields..
		mapSerializedField(prefabEntity);
	}

	for (entt::entity child : entityData->children) {
		updateFromPrefabInstance(child, prefabId);
	}
}

void PrefabManager::mapSerializedField(entt::entity entity) {
	Scripts* scripts = prefabRegistry.try_get<Scripts>(entity);
	Button* button  = prefabRegistry.try_get<Button>(entity);

	if (scripts) {
		for (ScriptData& scriptDatas : scripts->scriptDatas) {
			for (FieldData& fields : scriptDatas.fields) {
				std::visit([&](auto&& value) {
					using Type = std::decay_t<decltype(value)>;

					if constexpr (std::same_as<Type, entt::entity>) {
						remapEntityId(value);
					}

				}, fields.data);
			}
		}
	}


	if (button) {
		remapEntityId(button->reference.entity);
	}
}

void PrefabManager::remapEntityId(entt::entity& ecsEntityId) {
	EntityData const* entityData = ecs.registry.try_get<EntityData>(ecsEntityId);

	if (!entityData) {
		return;
	}

	auto iterator = entityGUIDToPrefabEntity.find(entityData->entityGUID);

	if (iterator != entityGUIDToPrefabEntity.end()) {
		ecsEntityId = iterator->second;
	}
}
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

void PrefabManager::mapSerializedField(entt::entity entity, std::unordered_map<entt::entity, entt::entity> map) {

	auto* scripts = ecsRegistry.try_get<Scripts>(entity);
	if (scripts != nullptr) {
		for (ScriptData& scriptDatas : scripts->scriptDatas) {
			for (FieldData& fields : scriptDatas.fields) {
				std::visit([&](auto&& value) {
					using Type = std::decay_t<decltype(value)>;

					if constexpr (std::same_as<Type, entt::entity>) {
						auto iterator = map.find(value);
						if (iterator != map.end()) {
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

			//checks if prefab is loaded
			if (prefabMap.find(entityData->prefabMetaData.prefabID) == prefabMap.end()) {
				loadPrefab(entityData->prefabMetaData.prefabID);
			}
			
			//only broadcast the root
			EntityData* prefabData = prefabRegistry.try_get<EntityData>(prefabMap[entityData->prefabID]);
			if (prefabData->name == entityData->name) {
				broadcast(entityData->prefabMetaData.prefabEntity);
			}
		}
	}
}

template<typename ...Components>
void PrefabManager::updateComponents(entt::registry& ecsRegistry, entt::registry& prefabRegistry, entt::entity entity, entt::entity prefabEntity) {

	([&]() {
		if (!std::is_same<EntityData, Components>::value) {
			auto* component = prefabRegistry.try_get<Components>(prefabEntity);

			EntityData* ecsEntityData = ecsRegistry.try_get<EntityData>(entity);
			bool overrideCheck{ false };

			//check for override check box
			if (ecsEntityData->overridenComponents.find(Family::id<Components>()) != ecsEntityData->overridenComponents.end()) {
				if (!ecsEntityData->overridenComponents[Family::id<Components>()]) {
					overrideCheck = true;
				}
			}
			if (component && overrideCheck) {
				auto* entityComponent = ecsRegistry.try_get<Components>(entity);
				*entityComponent = *component;
			}
		}
		}(), ...);
}
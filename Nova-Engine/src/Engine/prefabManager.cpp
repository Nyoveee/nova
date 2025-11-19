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
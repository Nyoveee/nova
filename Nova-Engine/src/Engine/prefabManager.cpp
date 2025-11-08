#include "prefabManager.h"
#include "Serialisation/serialisation.h"
#include "component.h"
#include "engine.h"

PrefabManager::PrefabManager(Engine& engine) :
	resourceManager { engine.resourceManager },
	ecsRegistry		{ engine.ecs.registry },
	ecs				{ engine.ecs}
{}

entt::registry& PrefabManager::getPrefabRegistry()
{
	return prefabRegistry;
}

std::unordered_map<ResourceID, entt::entity> PrefabManager::getPrefabMap()
{
	return prefabMap;
}

entt::entity PrefabManager::loadPrefab(ResourceID id) {
	auto&& [resource, result] = resourceManager.getResource<Prefab>(id);
	const char* fileName = resource->getFilePath().string.c_str();

	entt::entity entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id), prefabRegistry);
	prefabMap[id] = entity;

	return entity;
}
#pragma once

#include <entt/entt.hpp>
#include "resource.h"
#include "Serialisation/serialisation.h"
#include "export.h"

class ECS;
class Engine;
class ResourceManager;

class PrefabManager {
public:
	PrefabManager(Engine& engine);
	ENGINE_DLL_API entt::registry& getPrefabRegistry();
	ENGINE_DLL_API std::unordered_map<ResourceID, entt::entity> getPrefabMap();

public:
	template<typename ...Components>
	entt::entity instantiatePrefab(ResourceID id);

	template<typename ...Components>
	void updateComponents(entt::registry& ecsRegistry, entt::registry& prefabRegistry, entt::entity entity, entt::entity prefabEntity);
	
	ENGINE_DLL_API entt::entity loadPrefab(ResourceID id);
	ENGINE_DLL_API void mapSerializedField(entt::entity entity, std::unordered_map<entt::entity, entt::entity> map);
	ENGINE_DLL_API void broadcast(entt::entity prefabEntity);
	ENGINE_DLL_API void prefabBroadcast(); 

private:

	template<typename ...Components>
	entt::entity instantiatePrefabRecursive(entt::entity prefabEntity);

private:
	entt::registry prefabRegistry;
	std::unordered_map<ResourceID, entt::entity> prefabMap;
	ResourceManager& resourceManager;
	entt::registry& ecsRegistry;
	ECS& ecs;
	
	//maps a prefab entity to a ecs entity
	std::unordered_map<entt::entity, entt::entity> map;
};

#include "prefabManager.ipp"


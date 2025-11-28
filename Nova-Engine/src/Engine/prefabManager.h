#pragma once

#include <entt/entt.hpp>
#include "resource.h"
#include "Serialisation/serialisation.h"
#include "export.h"

class ECS;
class Engine;
class ResourceManager;

// Entity ID that exist in the prefab registry instead.
using PrefabEntityID = entt::entity;

// =================================================================
// Dealing with entity ids..
// 
// We need to effectively map entity ids from different domains.
// 1. Entity ID in the prefab file itself [PrefabFileEntityID]
// 2. Entity ID in the prefab registry, loaded in memory when reading from prefab files..
// 3. When instantiating a prefab, we need to generate a new appropriate entity ID in the main ECS registry (a prefab instance)
// 
// The main issue that comes with this is maintaining entity id hierarchy invariant.
// 
// 1. Maintaining entity hierarchy when mapping from prefab file entity id to prefab registry entity id.
// 
// =================================================================
class PrefabManager {
public:
	ENGINE_DLL_API PrefabManager(Engine& engine);

	ENGINE_DLL_API entt::registry& getPrefabRegistry();
	ENGINE_DLL_API std::unordered_map<ResourceID, entt::entity> const& getPrefabMap() const;

public:
	// Creates a whole prefab instance given prefab id. May fail.
	ENGINE_DLL_API entt::entity instantiatePrefab(ResourceID id);

	template<typename ...Components>
	void updateComponents(entt::registry& toRegistry, entt::registry& fromRegistry, entt::entity entity, entt::entity prefabEntity);
	
	ENGINE_DLL_API PrefabEntityID loadPrefab(ResourceID id);

	ENGINE_DLL_API void mapSerializedField(entt::entity entity, std::unordered_map<PrefabEntityID, entt::entity> const& entityMapping);
	ENGINE_DLL_API void broadcast(entt::entity prefabEntity);
	ENGINE_DLL_API void prefabBroadcast(); 
	ENGINE_DLL_API entt::entity getParent(entt::entity prefabInstance);
	ENGINE_DLL_API void updateFromPrefabInstance(entt::entity prefabInstance);
	ENGINE_DLL_API void updatePrefab(entt::entity prefabInstance);
	ENGINE_DLL_API void convertToPrefab(entt::entity entity, ResourceID id);

private:

	template<typename ...Components>
	entt::entity instantiatePrefabRecursive(PrefabEntityID prefabEntity);

private:
	entt::registry prefabRegistry;
	std::unordered_map<ResourceID, PrefabEntityID> prefabMap;

	ResourceManager& resourceManager;
	entt::registry& ecsRegistry;
	ECS& ecs;
	
	// maps a prefab entity to a ecs entity
	std::unordered_map<PrefabEntityID, entt::entity> prefabEntityIdToInstanceId;
};

#include "prefabManager.ipp"


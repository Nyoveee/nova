#pragma once

#include <fstream>
#include <entt/entt.hpp>
#include <json/json.hpp>
#include "export.h"
#include "type_alias.h"
#include "config.h"
#include "resource.h"

class ECS;

class Prefab;
using Json = nlohmann::json;
using PrefabEntityID = entt::entity;
using PrefabFileEntityID = entt::entity;

namespace Serialiser {
	// ======================================================================================================
	FRAMEWORK_DLL_API void serialiseScene(entt::registry& registry, std::vector<Layer> const& layer, const char* fileName);
	FRAMEWORK_DLL_API void deserialiseScene(entt::registry& registry, std::vector<Layer>& layers, const char* fileName);

	FRAMEWORK_DLL_API GameConfig deserialiseGameConfig(const char* fileName);
	FRAMEWORK_DLL_API void serialiseGameConfig(const char* fileName, GameConfig const& config);

	FRAMEWORK_DLL_API void serialiseEditorConfig(const char* fileName, bool consol, bool debugUi, bool hierarchy, bool componentInspector);
	FRAMEWORK_DLL_API void deserialiseEditorConfig(const char* fileName);

	// ======================================================================================================
	// Dealing with de/serialisation with prefabs.

	// Given the prefab file path, this loads all the details of into the prefab registry, and returning the root entity for easy instantiation in the future.
	// Also returns the mapping of prefab file entity to prefab entity.
	FRAMEWORK_DLL_API PrefabEntityID deserialisePrefab(const char* filepath, ResourceID prefabResourceId, entt::registry& prefabRegistry, std::unordered_map<PrefabFileEntityID, PrefabEntityID>& mapping);
	
	// Deserialise a given prefab file entity ID in the json file (json file is represented by `jsonVectorOfEntities`), and returns the mapped prefab entity id.
	FRAMEWORK_DLL_API PrefabEntityID deserialisePrefabRecursive(std::vector<Json> const& jsonVectorOfEntities, PrefabFileEntityID prefabFileEntityID, entt::registry& prefabRegistry, ResourceID prefabResourceId, std::unordered_map<PrefabFileEntityID, PrefabEntityID>& mapping);
	
	FRAMEWORK_DLL_API void serialisePrefab(entt::registry& registry, entt::entity entity, std::ofstream& file);
	FRAMEWORK_DLL_API void serialisePrefabRecursive(entt::registry& registry, entt::entity entity, std::vector<Json>& jsonVec, bool checkParent);
	
	FRAMEWORK_DLL_API entt::id_type findLargestEntity(entt::registry& registry);

	template <typename ...Components>
	Json serialiseComponents(entt::registry& registry, entt::entity entity);

	template <typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, Json en);

	template <typename T>
	Json serialiseComponent(T& component);

	template <typename T>
	void deserialiseComponent(Json jsonComponent, entt::registry& registry, entt::entity entity);

	// =============================================
	// General purpose reflection serialization.
	// =============================================
	template <typename T>
	void serializeToJsonFile(T const& data, std::ofstream& file);

	template <typename T>
	void deserializeFromJsonFile(T const& data, std::ifstream& file);
};

#include "serialisation.ipp"
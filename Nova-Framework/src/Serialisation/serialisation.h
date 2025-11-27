#pragma once

#include <fstream>
#include <entt/entt.hpp>
#include <json/json.hpp>
#include "export.h"
#include "type_alias.h"
#include "config.h"
#include "resource.h"

class ECS;

using Json = nlohmann::json;

namespace Serialiser {
	FRAMEWORK_DLL_API void serialiseScene(entt::registry& registry, std::vector<Layer> const& layer, const char* fileName);

	FRAMEWORK_DLL_API void deserialiseScene(entt::registry& registry, std::vector<Layer>& layers, const char* fileName);

	FRAMEWORK_DLL_API GameConfig deserialiseGameConfig(const char* fileName);
	FRAMEWORK_DLL_API void serialiseGameConfig(const char* fileName, GameConfig const& config);

	FRAMEWORK_DLL_API void serialiseEditorConfig(const char* fileName, bool consol, bool debugUi, bool hierarchy, bool componentInspector);
	FRAMEWORK_DLL_API void deserialiseEditorConfig(const char* fileName);

	//FRAMEWORK_DLL_API entt::entity deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id);
	FRAMEWORK_DLL_API entt::entity deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id, entt::registry& prefabRegistry);
	FRAMEWORK_DLL_API void serialisePrefab(entt::registry& registry, entt::entity entity, std::ofstream& file, std::size_t id);
	FRAMEWORK_DLL_API void serialisePrefabRecursive(entt::registry& registry, entt::entity entity, std::vector<Json>& jsonVec, bool checkParent, std::size_t id);
	FRAMEWORK_DLL_API entt::id_type findLargestEntity(entt::registry& registry);

	template<typename ...Components>
	FRAMEWORK_DLL_API void deserialisePrefabRecursive(std::vector<Json> jsonVec, entt::entity& rootEntity, entt::registry& prefabRegistry, entt::id_type highestID, std::unordered_map<entt::entity, entt::entity>& map, std::size_t id);

	//FRAMEWORK_DLL_API void deserialisePrefabRecursive(std::vector<Json> jsonVec, int end, entt::registry& registry, entt::id_type highestID, std::vector<entt::entity>& childVec, std::size_t resourceid, entt::entity& rootEntity, entt::registry& prefabRegistry);
	//FRAMEWORK_DLL_API void deserialisePrefabRecursive(std::vector<Json> jsonVec, int index, entt::registry& registry, entt::id_type highestID, std::size_t resourceid, entt::entity& rootEntity, entt::registry& prefabRegistry, entt::entity child, std::unordered_map<entt::entity, entt::entity>& map);
	
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
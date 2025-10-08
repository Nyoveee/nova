#pragma once

#include <fstream>
#include <entt/entt.hpp>
#include <json/json.hpp>
#include "export.h"


class ECS;

using json = nlohmann::json;

namespace Serialiser {
	ENGINE_DLL_API void serialiseScene(ECS& ecs, const char* fileName);
	void deserialiseScene(ECS& ecs, const char* fileName);
	void serialiseGameConfig(const char* fileName, int gamewidth, int gameHeight);

	ENGINE_DLL_API void deserialiseGameConfig(const char* fileName, int& gameWidth, int& gameHeight, std::string& windowName);
	ENGINE_DLL_API void serialiseEditorConfig(const char* fileName, bool consol, bool debugUi, bool hierarchy, bool componentInspector);
	ENGINE_DLL_API void deserialiseEditorConfig(const char* fileName);

	ENGINE_DLL_API void deserialisePrefab(const char* fileName);
	ENGINE_DLL_API void serialisePrefab(entt::registry& registry, entt::entity entity);

	template <typename ...Components>
	json serialiseComponents(entt::registry& registry, entt::entity entity);

	template <typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, json en);
	//void deserialiseComponents(entt::registry& registry, entt::entity entity, std::ifstream& inputFile);

	template <typename T>
	json serialiseComponent(T& component);

	template <typename T>
	//void deserialiseComponent(std::ifstream& inputFile, json jsonComponent);
	void deserialiseComponent(json jsonComponent, entt::registry& registry, entt::entity entity);
};

#include "serialisation.ipp"
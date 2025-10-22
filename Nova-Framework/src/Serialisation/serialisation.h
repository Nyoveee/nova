#pragma once

#include <fstream>
#include <entt/entt.hpp>
#include <json/json.hpp>
#include "export.h"

using Json = nlohmann::json;

namespace Serialiser {
	FRAMEWORK_DLL_API void serialiseScene(entt::registry& registry, const char* fileName);

	FRAMEWORK_DLL_API void serialiseGameConfig(const char* fileName, int gamewidth, int gameHeight);
	FRAMEWORK_DLL_API void deserialiseScene(entt::registry& registry, const char* fileName);

	FRAMEWORK_DLL_API void deserialiseGameConfig(const char* fileName, int& gameWidth, int& gameHeight, std::string& windowName);
	FRAMEWORK_DLL_API void serialiseEditorConfig(const char* fileName, bool consol, bool debugUi, bool hierarchy, bool componentInspector);
	FRAMEWORK_DLL_API void deserialiseEditorConfig(const char* fileName);

	template <typename ...Components>
	Json serialiseComponents(entt::registry& registry, entt::entity entity);

	template <typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, Json en);

	template <typename T>
	Json serialiseComponent(T& component);

	template <typename T>
	void deserialiseComponent(Json jsonComponent, entt::registry& registry, entt::entity entity);

	template <typename T>
	void serializeToJsonFile(T const& data, std::ofstream& file);

	template <typename T>
	void deserializeFromJsonFile(T const& data, std::ifstream& file);
};

#include "serialisation.ipp"
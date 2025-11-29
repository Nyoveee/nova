#include <string>

#include "reflection.h"

#include "component.h"
#include "magic_enum.hpp"

#include "Logger.h"

#include "serializeToJson.h"
#include "deserializeFromJson.h"

namespace Serialiser {
	template<typename ...Components>
	Json serialiseComponents(entt::registry& registry, entt::entity entity) {
		Json componentsJson;

		([&]() {
			Components* component = registry.try_get<Components>(entity);

#if defined(_MSC_VER)
			// let's hope msvc doesnt change implementation haha
			// removes the `struct ` infront of type name.

			// 2 local variable of string because of lifetime :c
			std::string originalTypeName = typeid(Components).name();
			std::string typeName = originalTypeName.substr(7);
#else
			constexpr char const* name = typeid(Components).name();
#endif

			if (component) {
				Json componentJson = serialiseComponent(*component);
				componentsJson[typeName] = componentJson;
			}
		}(), ...);

		componentsJson["id"] = static_cast<unsigned>(entity);
		return componentsJson;
	}

	template<typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, Json en) {
		([&]() {
			deserialiseComponent<Components>(en, registry, entity);
		}(), ...);
	}

	template<typename T>
	Json serialiseComponent(T& component) {
		Json componentJson;

		// implement serialisation logic for each component.
		if constexpr (!reflection::isReflectable<T>()) {
			return componentJson;
		}

		componentJson = serializeToJson(component);
		return componentJson;
	}
	// this was the starting point
	template<typename T>
	void deserialiseComponent(Json jsonComponent, entt::registry& registry, entt::entity entity) {
		T component;

		std::string originalComponentName = typeid(component).name();
		std::string componentName = originalComponentName.substr(7);

		if (jsonComponent.find(componentName) == jsonComponent.end())
			return;

		try {
			reflection::visit([&](auto fieldData) {
				auto& dataMember = fieldData.get();
				constexpr const char* dataMemberName = fieldData.name();
				using DataMemberType = std::decay_t<decltype(dataMember)>;
				deserializeFromJson<DataMemberType>(dataMember, jsonComponent[componentName][dataMemberName]);

			}, component);

			registry.emplace<T>(entity, std::move(component));
		}
		catch (std::exception const& ex) {
			Logger::error("Error parsing {} for entity {} : {}", componentName, static_cast<unsigned int>(entity), ex.what());
			registry.emplace<T>(entity, std::move(component));
		}
	}

	template<typename T>
	void serializeToJsonFile(T const& data, std::ofstream& file) {
		file << std::setw(4) << serializeToJson(data) << std::endl;
	}
	
	template<typename T>
	void deserializeFromJsonFile(T& data, std::ifstream& file) {
		Json json;
		file >> json;
		deserializeFromJson(data, json);
	}
}
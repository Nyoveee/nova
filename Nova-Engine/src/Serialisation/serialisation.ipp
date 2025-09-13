#include <string>

#include "serialisation.h"
#include "ecs.h"
#include "Libraries/reflection.h"

#include "Component/component.h"

namespace Serialiser {
	template<class T>
	concept IsTypedAssetID = requires(T x) {
		{ TypedAssetID{ x } } -> std::same_as<T>;
	};

	template<typename ...Components>
	json serialiseComponents(entt::registry& registry, entt::entity entity) {
		json componentsJson;

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
				json componentJson = serialiseComponent(*component);
				componentsJson[typeName] = componentJson;
			}
		}(), ...);

		componentsJson["id"] = static_cast<unsigned>(entity);
		return componentsJson;
	}

	template<typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, std::ifstream& outputFile) {

	}

	template<typename T>
	json serialiseComponent(T& component) {
		json componentJson;

		// implement serialisation logic for each component.
		if constexpr (!reflection::isReflectable<T>()) {
			return componentJson;
		}
		else {
			reflection::visit([&](auto fieldData) {
				auto& dataMember = fieldData.get();
				constexpr const char* dataMemberName = fieldData.name();
				using DataMemberType = std::decay_t<decltype(dataMember)>;

				if constexpr (std::same_as<DataMemberType, glm::vec3>) {
					componentJson[dataMemberName]["x"] = dataMember.x;
					componentJson[dataMemberName]["y"] = dataMember.y;
					componentJson[dataMemberName]["z"] = dataMember.z;
				}

				else if constexpr (std::same_as<DataMemberType, glm::vec3>) {

				}

				else if constexpr (std::same_as<DataMemberType, Color>) {

				}

				else if constexpr (std::same_as<DataMemberType, glm::quat>) {

				}

				else if constexpr (std::same_as<DataMemberType, EulerAngles>) {

				}

				else if constexpr (std::same_as<DataMemberType, AssetID>) {

				}

				else if constexpr (IsTypedAssetID<DataMemberType>) {

				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {

				}

				else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {
					
				}

				// it's an enum. let's display a dropdown box for this enum.
				// how? using enum reflection provided by "magic_enum.hpp" :D
				else if constexpr (std::is_enum_v<DataMemberType>) {

				}

				else {
					// int, float, std::string,
					//componentJson[dataMemberName] = dataMember;
				}

			}, component);

			return componentJson;
		}
	}

	template<typename T>
	void deserialiseComponent(std::ifstream& outputFile)
	{

	}
}
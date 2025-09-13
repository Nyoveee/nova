#include <string>

#include "serialisation.h"
#include "ecs.h"
#include "Libraries/reflection.h"

#include "Component/component.h"
#include "Libraries/magic_enum.hpp"

#include "Logger.h"

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
	void deserialiseComponents(entt::registry& registry, entt::entity entity, json en) {
		// for each component (EntityData, Transform, etc.)
		([&]() {
			deserialiseComponent<Components>(en, registry, entity);
		}(), ...);
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

				else if constexpr (std::same_as<DataMemberType, entt::entity>) {
					componentJson[dataMemberName] = static_cast<unsigned int>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, entt::entity>) {
					componentJson[dataMemberName] = magic_enum::enum_name(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, Color>) {
					glm::vec3 vec = dataMember;
					componentJson[dataMemberName]["r"] = vec.x;
					componentJson[dataMemberName]["g"] = vec.y;
					componentJson[dataMemberName]["b"] = vec.z;
				}

				else if constexpr (std::same_as<DataMemberType, glm::quat>) {
					componentJson[dataMemberName]["w"] = dataMember.w;
					componentJson[dataMemberName]["x"] = dataMember.x;
					componentJson[dataMemberName]["y"] = dataMember.y;
					componentJson[dataMemberName]["z"] = dataMember.z;
				}

				else if constexpr (std::same_as<DataMemberType, EulerAngles>) {
					componentJson[dataMemberName]["x"] = dataMember.angles.x;
					componentJson[dataMemberName]["y"] = dataMember.angles.y;
					componentJson[dataMemberName]["z"] = dataMember.angles.z;
				}

				else if constexpr (std::same_as<DataMemberType, AssetID>) {
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (IsTypedAssetID<DataMemberType>) {
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
					std::vector<json> jVec;
					
					//for each material in the map
					for (auto a : dataMember) {

						json tempJson;

						auto pipeline = magic_enum::enum_name(a.second.renderingPipeline);

						tempJson["materialName"] = a.first;
						tempJson["renderingPipeline"] = pipeline;
						tempJson["ambient"] = a.second.ambient;

						if (pipeline == "PBR") {
							json tempJ;

							tempJ["roughness"] = std::get<Material::Config>(a.second.config).roughness;
							tempJ["metallic"] = std::get<Material::Config>(a.second.config).metallic;
							tempJ["occulusion"] = std::get<Material::Config>(a.second.config).occulusion;
							tempJson["config"] = tempJ;
							tempJ.clear();
						}

						if (pipeline == "PBR" || pipeline == "BlinnPhong") {
							if (!(a.second.normalMap).has_value()) {
								tempJson["normalMap"] = nullptr;
							}
							else {
								tempJson["normalMap"] = static_cast<size_t>(a.second.normalMap.value());
							}
						}

						if (std::holds_alternative<Color>(a.second.albedo)) {
							glm::vec3 vec = std::get<Color>(a.second.albedo);
							json tempJ;
							tempJ["r"] = vec.x;
							tempJ["g"] = vec.y;
							tempJ["b"] = vec.z;

							tempJson["albedo"] = tempJ;
							tempJ.clear();
						}
						jVec.push_back(tempJson);
					}
					componentJson[dataMemberName] = jVec;
				}

				else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {

				}

				// it's an enum. let's display a dropdown box for this enum.
				// how? using enum reflection provided by "magic_enum.hpp" :D
				else if constexpr (std::is_enum_v<DataMemberType>) {
					componentJson[dataMemberName] = magic_enum::enum_name(dataMember);
				}

				else {
					// int, float, std::string,
					componentJson[dataMemberName] = dataMember;
				}

			}, component);

			return componentJson;
		}
	}

	template<typename T>
	//void deserialiseComponent(std::ifstream& outputFile, json jsonComponent) {
	void deserialiseComponent(json jsonComponent, entt::registry& registry, entt::entity entity) {
		T component;

		std::string originalComponentName = typeid(component).name();
		std::string componentName = originalComponentName.substr(7);

		if (jsonComponent.find(componentName) == jsonComponent.end())
			return;

		reflection::visit([&](auto fieldData) {
			auto& dataMember = fieldData.get();
			constexpr const char* dataMemberName = fieldData.name();
			using DataMemberType = std::decay_t<decltype(dataMember)>;

			if constexpr (std::same_as<DataMemberType, glm::vec3>) {
				glm::vec3 vec{	jsonComponent[componentName][dataMemberName]["x"],
								jsonComponent[componentName][dataMemberName]["y"],
								jsonComponent[componentName][dataMemberName]["z"] };
				dataMember = vec;
			}

			else if constexpr (std::same_as<DataMemberType, entt::entity>) {
				try {
					unsigned int entityNum = jsonComponent[componentName][dataMemberName];
					entt::entity entity = entityNum == std::numeric_limits<unsigned int>::max() ? entt::null : static_cast<entt::entity>(entityNum);
					dataMember = entity;
				}
				catch (std::exception const& ex) {
					Logger::warn("Failed to parse entity data member. {}", ex.what());
					dataMember = entt::null;
				}
			}

			else if constexpr (std::same_as<DataMemberType, std::vector<entt::entity>>) {
				std::vector<entt::entity> vec;
				for (auto a : jsonComponent[componentName][dataMemberName])
				{
					vec.push_back(static_cast<entt::entity>(a));
				}
				dataMember = vec;
			}

			else if constexpr (std::same_as<DataMemberType, Color>) {
				glm::vec3 vec{	jsonComponent[componentName][dataMemberName]["r"],
								jsonComponent[componentName][dataMemberName]["g"],
								jsonComponent[componentName][dataMemberName]["b"] };
				dataMember = vec;
			}

			else if constexpr (std::same_as<DataMemberType, glm::quat>) {
				glm::quat vec{	jsonComponent[componentName][dataMemberName]["w"],
								jsonComponent[componentName][dataMemberName]["x"],
								jsonComponent[componentName][dataMemberName]["y"],
								jsonComponent[componentName][dataMemberName]["z"] };
				dataMember = vec;
			}

			else if constexpr (std::same_as<DataMemberType, EulerAngles>) {
				glm::vec3 vec{	jsonComponent[componentName][dataMemberName]["x"],
								jsonComponent[componentName][dataMemberName]["y"],
								jsonComponent[componentName][dataMemberName]["z"] };
				dataMember = vec;
			}

			else if constexpr (std::same_as<DataMemberType, AssetID>) {
				dataMember = static_cast<AssetID>((jsonComponent[componentName][dataMemberName]));
			}

			else if constexpr (IsTypedAssetID<DataMemberType>) {
				using OriginalAssetType = DataMemberType::AssetType;

				dataMember = TypedAssetID<OriginalAssetType>{ static_cast<std::size_t>(jsonComponent[componentName][dataMemberName]) };
			}

			else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
				std::unordered_map<MaterialName, Material> map;
				//material and model id
				for (auto a : jsonComponent[componentName]["materials"]) {
					Material m;
					m.ambient = a["ambient"];

					std::string str = a["renderingPipeline"].dump();
					str = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));

					std::optional<Material::Pipeline> temp = magic_enum::enum_cast<Material::Pipeline>(str.c_str());
					if (temp.has_value()) {
						m.renderingPipeline = temp.value();
					}

					if (a["renderingPipeline"] == "BlinnPhong" || a["renderingPipeline"] == "PBR") {
						if (a["normalMap"] != nullptr) {
							m.normalMap = static_cast<AssetID>(a["normalMap"]);
						}
						else {
							m.normalMap = std::nullopt;
						}
					}

					if (a["renderingPipeline"] == "PBR") {
						Material::Config c;
						c.roughness = a["config"]["roughness"];
						c.metallic = a["config"]["metallic"];
						c.occulusion = a["config"]["occulusion"];
						m.config = c;
					}

					if (a.find("albedo") != a.end()) {
						glm::vec3 colorVec = { a["albedo"]["r"], a["albedo"]["g"] , a["albedo"]["b"] };
						m.albedo = colorVec;
					}

					map[a["materialName"]] = m;
				}
				dataMember = map;
			}

			else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {

			}

			// it's an enum. let's display a dropdown box for this enum.
			// how? using enum reflection provided by "magic_enum.hpp" :D
			else if constexpr (std::is_enum_v<DataMemberType>) {
 
				std::string str = jsonComponent[componentName][dataMemberName].dump();
				str = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));

				std::optional<DataMemberType> temp = magic_enum::enum_cast<DataMemberType>(str.c_str());

				if (temp.has_value()) {
					dataMember = temp.value();
				}
			}

			else {
				// int, float, std::string,
				//componentJson[dataMemberName] = dataMember;
				dataMember = jsonComponent[componentName][dataMemberName];
			}

			}, component);
		registry.emplace<T>(entity, std::move(component));

	}
}
#include <string>

#include "serialisation.h"
#include "ECS/ECS.h"
#include "reflection.h"

#include "component.h"
#include "magic_enum.hpp"

#include "Logger.h"

#include "Engine/ScriptingAPIManager.h"

#undef max
#undef min

namespace Serialiser {
	template<class T>
	concept IsTypedResourceID = requires(T x) {
		{ TypedResourceID{ x } } -> std::same_as<T>;
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

				else if constexpr (std::same_as<DataMemberType, Radian>) {
					componentJson[dataMemberName] = static_cast<float>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, Degree>) {
					componentJson[dataMemberName] = static_cast<float>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, ResourceID>) {
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (IsTypedResourceID<DataMemberType>) {
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
					std::vector<json> jVec;
					
					//for each material in the map
					for (auto&& [materialName, material] : dataMember) {

						json tempJson;

						tempJson["materialName"] = materialName;
						tempJson["renderingPipeline"] = magic_enum::enum_name(material.renderingPipeline);
						tempJson["ambient"] = material.ambient;

						switch (material.renderingPipeline)
						{
						case Material::Pipeline::PBR: {
							std::visit([&](auto&& config) {
								using Type = std::decay_t<decltype(config)>;

								if constexpr (std::same_as<Type, ResourceID>) {
									tempJson["config"] = static_cast<std::size_t>(config);
								}
								else /* it's config */ {
									json tempJ;

									tempJ["roughness"] = config.roughness;
									tempJ["metallic"] = config.metallic;
									tempJ["occulusion"] = config.occulusion;
									tempJson["config"] = tempJ;
								}
							}, material.config);
						}
						[[fallthrough]];
						case Material::Pipeline::BlinnPhong:
							if (!material.normalMap) {
								tempJson["normalMap"] = nullptr;
							}
							else {
								tempJson["normalMap"] = static_cast<size_t>(material.normalMap.value());
							}

							break;
						}

						// Handling albedo..
						std::visit([&](auto&& albedo) {
							using T = std::decay_t<decltype(albedo)>;

							json albedoJson;
							if constexpr (std::same_as<T, ResourceID>) {
								albedoJson["texture"] = static_cast<size_t>(albedo);
							}
							else /* its color */ {
								json tempJ;
								tempJ["r"] = albedo.r();
								tempJ["g"] = albedo.g();
								tempJ["b"] = albedo.b();
								albedoJson["color"] = tempJ;
							}

							tempJson["albedo"] = albedoJson;
						}, material.albedo);
						
						jVec.push_back(tempJson);
					}
					componentJson[dataMemberName] = jVec;
				}

				else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {
					json scriptArray;

					for (auto&& scriptData : dataMember) {
						json scriptJson;
						json fieldDataArray;

						for (auto&& field : scriptData.fields) {
							json fieldDataJson;
							fieldDataJson["name"] = field.name;
							
							std::visit([&](auto&& data) {
								using Type = std::decay_t<decltype(data)>;

								if constexpr (std::same_as<Type, glm::vec2>) {
									fieldDataJson["value"]["x"] = data.x;
									fieldDataJson["value"]["y"] = data.y;
								}
								else if constexpr (std::same_as<Type, glm::vec3>) {
									fieldDataJson["value"]["x"] = data.x;
									fieldDataJson["value"]["y"] = data.y;
									fieldDataJson["value"]["z"] = data.z;
								}
								else if constexpr (std::same_as<Type, entt::entity>) {
									fieldDataJson["value"]["entityId"] = static_cast<std::size_t>(data);
								}
								else if constexpr (std::is_fundamental_v<Type>) {
									fieldDataJson["value"] = data;
								}
								else {
									[] <bool flag = true> {
										static_assert(flag, "Attempting to serialise unsupported field type..");
									}();
								}

								fieldDataJson["type"] = Family::id<Type>();
							}, field.data);

							fieldDataArray.push_back(std::move(fieldDataJson));
						}

						scriptJson["fields"] = std::move(fieldDataArray);
						scriptJson["id"] = static_cast<std::size_t>(scriptData.scriptId);
						scriptArray.push_back(std::move(scriptJson));
					}

					componentJson[dataMemberName] = std::move(scriptArray);
				}
				
				else if constexpr (std::same_as<DataMemberType, std::unordered_map<std::string, AudioData>>) {
					json audioArray;

					for (auto&& [name, audioData] : dataMember) {
						json audioComponentJson;
						audioComponentJson["name"] = name;

						json audioDataJson;
						audioDataJson["id"] = static_cast<std::size_t>(audioData.AudioId);
						audioDataJson["volume"] = audioData.Volume;

						audioComponentJson["audioData"] = audioDataJson;

						audioArray.push_back(std::move(audioComponentJson));
					}

					componentJson[dataMemberName] = std::move(audioArray);
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
	// this was the starting point
	template<typename T>
	//void deserialiseComponent(std::ifstream& outputFile, json jsonComponent) {
	void deserialiseComponent(json jsonComponent, entt::registry& registry, entt::entity entity) {
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

				if constexpr (std::same_as<DataMemberType, glm::vec3>) {
					glm::vec3 vec{ jsonComponent[componentName][dataMemberName]["x"],
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
					glm::vec3 vec{ jsonComponent[componentName][dataMemberName]["r"],
									jsonComponent[componentName][dataMemberName]["g"],
									jsonComponent[componentName][dataMemberName]["b"] };
					dataMember = vec;
				}

				else if constexpr (std::same_as<DataMemberType, glm::quat>) {
					glm::quat vec{ jsonComponent[componentName][dataMemberName]["w"],
									jsonComponent[componentName][dataMemberName]["x"],
									jsonComponent[componentName][dataMemberName]["y"],
									jsonComponent[componentName][dataMemberName]["z"] };
					dataMember = vec;
				}

				else if constexpr (std::same_as<DataMemberType, EulerAngles>) {
					glm::vec3 vec{ jsonComponent[componentName][dataMemberName]["x"],
									jsonComponent[componentName][dataMemberName]["y"],
									jsonComponent[componentName][dataMemberName]["z"] };
					dataMember = vec;
				}

				else if constexpr (std::same_as<DataMemberType, Radian>) {
					float angle = jsonComponent[componentName][dataMemberName];
					dataMember = Radian{ angle };
				}

				else if constexpr (std::same_as<DataMemberType, Degree>) {
					float angle = jsonComponent[componentName][dataMemberName];
					dataMember = Degree{ angle };
				}

				else if constexpr (std::same_as<DataMemberType, ResourceID>) {
					dataMember = static_cast<ResourceID>((jsonComponent[componentName][dataMemberName]));
				}

				else if constexpr (IsTypedResourceID<DataMemberType>) {
					using OriginalAssetType = DataMemberType::AssetType;

					dataMember = TypedResourceID<OriginalAssetType>{ static_cast<std::size_t>(jsonComponent[componentName][dataMemberName]) };
				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
					std::unordered_map<MaterialName, Material> map;
					//material and model id
					for (auto json : jsonComponent[componentName]["materials"]) {
						Material material;
						material.ambient = json["ambient"];

						std::string str = json["renderingPipeline"].dump();
						str = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));

						std::optional<Material::Pipeline> temp = magic_enum::enum_cast<Material::Pipeline>(str.c_str());
						if (temp.has_value()) {
							material.renderingPipeline = temp.value();
						}

						switch (material.renderingPipeline)
						{
						case Material::Pipeline::PBR: {
							// its resource id..
							if (json["config"].is_number_unsigned()) {
								std::size_t resourceId = json["config"];
								material.config = ResourceID{ resourceId };
							}
							// its config...
							else {
								Material::Config config;

								config.roughness	= json["config"]["roughness"];
								config.metallic		= json["config"]["metallic"];
								config.occulusion	= json["config"]["occulusion"];

								material.config = config;
							}
						}
						[[fallthrough]];
						case Material::Pipeline::BlinnPhong:
							if (json["normalMap"].is_null()) {
								material.normalMap = std::nullopt;
							}
							else {
								material.normalMap = static_cast<ResourceID>(json["normalMap"]);
							}

						[[fallthrough]];
						case Material::Pipeline::Color:
							if (json["albedo"].find("color") != json["albedo"].end()) {
								glm::vec3 colorVec = { json["albedo"]["color"]["r"], json["albedo"]["color"]["g"] , json["albedo"]["color"]["b"] };
								material.albedo = colorVec;
							}
							else {
								std::size_t id = json["albedo"]["texture"];
								material.albedo = id;
							}

							break;
						}

						map[json["materialName"]] = material;
					}
					dataMember = map;
				}

				else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {
					std::vector<ScriptData> allScripts;

					// parsing every script...
					for (auto&& scriptJson : jsonComponent[componentName]["scriptDatas"]) {
						// for each script, they have an id and a field..
						std::vector<FieldData> fields;

						// let's parse all fields of a given script..
						for (auto&& fieldJson : scriptJson["fields"]) {
							FieldData scriptFieldData;
							scriptFieldData.name = fieldJson["name"];

							// we check and parse the different possible types of each field data..
							auto parseVariantJson = [&]<typename ...Types>() {
								([&]() {
									if (Family::id<Types>() == fieldJson["type"]) {
										if constexpr (std::same_as<Types, glm::vec2>) {
											glm::vec2 vec2;
											vec2.x = fieldJson["value"]["x"];
											vec2.y = fieldJson["value"]["y"];
											scriptFieldData.data = vec2;
										}
										else if constexpr (std::same_as<Types, glm::vec3>) {
											glm::vec3 vec3;
											vec3.x = fieldJson["value"]["x"];
											vec3.y = fieldJson["value"]["y"];
											vec3.z = fieldJson["value"]["z"];
											scriptFieldData.data = vec3;
										}
										else if constexpr (std::same_as<Types, entt::entity>) {
											unsigned id = fieldJson["value"]["entityId"];
											scriptFieldData.data = static_cast<entt::entity>(id);

										}
										else if constexpr (std::is_fundamental_v<Types>) {
											Types data = fieldJson["value"];
											scriptFieldData.data = data;
										}
										else {
											[] <bool flag = true> {
												static_assert(flag, "Attempting to Deserialise unsupported field type..");
											}();
										}
									}
								}(), ...);
							};

							parseVariantJson.template operator()<ALL_FIELD_TYPES>();
							fields.push_back(std::move(scriptFieldData));
						}

						// retrieve script id..
						std::size_t id = scriptJson["id"];
						allScripts.push_back(ScriptData{ TypedResourceID<ScriptAsset>{ id }, std::move(fields) });
					}

					dataMember = std::move(allScripts);
				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<std::string, AudioData>>) {
					for (auto&& audioComponent : jsonComponent[componentName][dataMemberName]) {
						std::string name = audioComponent["name"];
					
						std::size_t id = audioComponent["audioData"]["id"];
						float volume = audioComponent["audioData"]["volume"];

						dataMember.insert({ std::move(name), AudioData{ id, volume } });
					}
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
		catch (std::exception const& ex) {
			Logger::error("Error parsing {} for entity {} : {}", componentName, static_cast<unsigned int>(entity), ex.what());
		}
	}
}
#include <string>

#include "serialisation.h"

#include "reflection.h"

#include "component.h"
#include "magic_enum.hpp"

#include "Logger.h"

#include "Engine/ScriptingAPIManager.h"
#include "serializeProperty.h"

#undef max
#undef min

namespace Serialiser {
	template<class T>
	concept IsTypedResourceID = requires(T x) {
		{ TypedResourceID{ x } } -> std::same_as<T>;
	};

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
		// for each component (EntityData, Transform, etc.)
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

		reflection::visit([&](auto fieldData) {
			auto& dataMember = fieldData.get();
			constexpr const char* dataMemberName = fieldData.name();
			using DataMemberType = std::decay_t<decltype(dataMember)>;
			SerializeProperty<DataMemberType>(componentJson, dataMemberName, dataMember);
		}, component);

		return componentJson;
	}
	// this was the starting point
	template<typename T>
	//void deserialiseComponent(std::ifstream& outputFile, json jsonComponent) {
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
											unsigned id = fieldJson["value"];
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
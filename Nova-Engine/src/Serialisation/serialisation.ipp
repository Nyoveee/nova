#include <string>

#include "serialisation.h"
#include "ecs.h"
#include "Libraries/reflection.h"

#include "Component/component.h"
#include "Libraries/magic_enum.hpp"


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
	void deserialiseComponents(entt::registry& registry, entt::entity entity, std::ifstream& inputFile, json en) {
		// for each component (EntityData, Transform, etc.)
		//en[componentName]
		//for (auto j : en.items())
		//{
		//	j.key()
		//	//deserialiseComponent(j);
		//}
		// 
		
		//std::cout<<"testing" << en.size();
		std::cout << ">>> entered deserialiseComponents\n";
		// manual run to test this works 
	   //	deserialiseComponent<Transform>(en);
	   //deserialiseComponent<Rigidbody>(en);
		

		([&]() {
			std::cout << "Expanding component...\n";
			//deserialiseComponent<Components>(registry, entity, test, en);
			    deserialiseComponent<Components>(en); 
			//deserialiseComponent<Components>(registry, entity, en);
			
		}(), ...);

		std::cout << "Components pack size = " << sizeof...(Components) << "\n";
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
				//std::cout << "test " << dataMemberName << std::endl;
				using DataMemberType = std::decay_t<decltype(dataMember)>;

				if constexpr (std::same_as<DataMemberType, glm::vec3>) {
					componentJson[dataMemberName]["x"] = dataMember.x;
					componentJson[dataMemberName]["y"] = dataMember.y;
					componentJson[dataMemberName]["z"] = dataMember.z;
				}

				//else if constexpr (std::same_as<DataMemberType, glm::vec3>) {

				//}

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
					//auto id = dataMember;
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (IsTypedAssetID<DataMemberType>) {
					componentJson[dataMemberName] = static_cast<size_t>(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
					std::vector<json> jVec;
					for (auto&& [name, material] : dataMember) {
						//componentJson[dataMemberName] = magic_enum::enum_name(material);

						json tempJson;
						tempJson["renderingPipeline"] = magic_enum::enum_name(material.renderingPipeline);
						jVec.push_back(tempJson);
						tempJson.clear();

						//tempJson[name] = material.;
						//jVec.push_back(tempJson);
						//tempJson.clear();
					}
					componentJson[dataMemberName] = jVec;
				}

				else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {
					std::vector<json> jVec;
					json tempJson;

				/*	for (ScriptData s : dataMember) {
						tempJson[dataMemberName] = s.name;
						jVec.push_back(tempJson);
						tempJson.clear();
					}*/

					componentJson[dataMemberName] = jVec;
				}

				// it's an enum. let's display a dropdown box for this enum.
				// how? using enum reflection provided by "magic_enum.hpp" :D
				else if constexpr (std::is_enum_v<DataMemberType>) {
					componentJson[dataMemberName] = magic_enum::enum_name(dataMember);
				}

				else if constexpr (std::same_as<DataMemberType, entt::entity>) {
					componentJson[dataMemberName] = magic_enum::enum_name(dataMember.parent);
				}


				else {
					// int, float, std::string,
					//componentJson[dataMemberName] = dataMember;
				}

			}, component);

			return componentJson;
		}
	}
	// this was the starting point
	template<typename T>
	//void deserialiseComponent(std::ifstream& outputFile, json jsonComponent) {
	void deserialiseComponent(json jsonComponent) {
		T component;
		std::cout << typeid(component).name() << std::endl;

		//auto& dataMember = jsonComponent;
		////if(dataMember.nam)
		reflection::visit([&](auto fieldData) {

			}, component);
	}

	//template<typename T>
	//void deserialiseComponent(entt::registry& registry, entt::entity entity, const json& en) {
	//	// Check if the JSON contains this component
	//	if (!en.contains(T::GetTypeName())) 
	//		return;

	//	const json& jsonComponent = en[T::GetTypeName()];
	//	T component;

	//	std::cout << "Deserialising component: " << T::GetTypeName() << std::endl;

	//	// Use reflection to populate component fields from JSON
	//	reflection::visit([&](auto& fieldData, auto& fieldName) {
	//		if (jsonComponent.contains(fieldName)) { 
	//			fieldData = jsonComponent[fieldName].get<std::decay_t<decltype(fieldData)>>();
	//		}
	//		}, component);

	//	// Finally, attach component to entity
	//	registry.emplace<T>(entity, component);
	//}

}
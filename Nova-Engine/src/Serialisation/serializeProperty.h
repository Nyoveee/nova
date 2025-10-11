#pragma once
#include "type_concepts.h"
#include "ECS/ECS.h"
#include "magic_enum.hpp"

#include <json/json.hpp>

using Json = nlohmann::json;

/***************************************************************************************
	Base
****************************************************************************************/
template<typename DataMemberType>
inline void SerializeProperty(Json& jsonComponent,const char* dataMemberName, DataMemberType const& dataMember) {
	(void)jsonComponent, dataMemberName, dataMember;
}
/***************************************************************************************
	Excluded List, See type_concepts.h
****************************************************************************************/
template<NonSerializableTypes DataMemberType>
inline void SerializeProperty(Json& jsonComponent, const char* dataMemberName, DataMemberType const& dataMember) {
	(void)jsonComponent, dataMemberName, dataMember;
}
/***************************************************************************************
	Special Constraints
****************************************************************************************/
template<IsTypedResourceID DataMemberType>
inline void SerializeProperty(Json& jsonComponent, const char* dataMemberName, DataMemberType const& dataMember) {
	jsonComponent[dataMemberName] = static_cast<size_t>(dataMember);
}
template<IsFundamental DataMemberType>
inline void SerializeProperty(Json& jsonComponent, const char* dataMemberName, DataMemberType const& dataMember) {
	jsonComponent[dataMemberName] = dataMember;
}
template<IsEnum DataMemberType>
inline void SerializeProperty(Json& jsonComponent, const char* dataMemberName, DataMemberType const& dataMember) {
	jsonComponent[dataMemberName] = magic_enum::enum_name(dataMember);
}
/***************************************************************************************
	Listings
****************************************************************************************/
template<>
inline void SerializeProperty<glm::vec2>(Json& jsonComponent, const char* dataMemberName, glm::vec2 const& dataMember) {
	jsonComponent[dataMemberName]["x"] = dataMember.x;
	jsonComponent[dataMemberName]["y"] = dataMember.y;
}
template<>
inline void SerializeProperty<glm::vec3>(Json& jsonComponent, const char* dataMemberName, glm::vec3 const& dataMember) {
	jsonComponent[dataMemberName]["x"] = dataMember.x;
	jsonComponent[dataMemberName]["y"] = dataMember.y;
	jsonComponent[dataMemberName]["z"] = dataMember.z;
}
template<>
inline void SerializeProperty<entt::entity>(Json& jsonComponent, const char* dataMemberName, entt::entity const& dataMember) {
	jsonComponent[dataMemberName] = static_cast<unsigned int>(dataMember);
}
template<>
inline void SerializeProperty<std::vector<entt::entity>>(Json& jsonComponent, const char* dataMemberName, std::vector<entt::entity> const& dataMember) {
	jsonComponent[dataMemberName] = dataMember;
}
template<>
inline void SerializeProperty<Color>(Json& jsonComponent, const char* dataMemberName, Color const& dataMember) {
	jsonComponent[dataMemberName]["r"] = dataMember.r();
	jsonComponent[dataMemberName]["g"] = dataMember.g();
	jsonComponent[dataMemberName]["b"] = dataMember.b();
}
template<>
inline void SerializeProperty<glm::quat>(Json& jsonComponent, const char* dataMemberName, glm::quat const& dataMember) {
	jsonComponent[dataMemberName]["w"] = dataMember.w;
	jsonComponent[dataMemberName]["x"] = dataMember.x;
	jsonComponent[dataMemberName]["y"] = dataMember.y;
	jsonComponent[dataMemberName]["z"] = dataMember.z;
}
template<>
inline void SerializeProperty<EulerAngles>(Json& jsonComponent, const char* dataMemberName, EulerAngles const& dataMember) {
	jsonComponent[dataMemberName]["x"] = dataMember.angles.x;
	jsonComponent[dataMemberName]["y"] = dataMember.angles.y;
	jsonComponent[dataMemberName]["z"] = dataMember.angles.z;
}
template<>
inline void SerializeProperty<Radian>(Json& jsonComponent, const char* dataMemberName, Radian const& dataMember) {
	jsonComponent[dataMemberName] = static_cast<float>(dataMember);
}
template<>
inline void SerializeProperty<Degree>(Json& jsonComponent, const char* dataMemberName, Degree const& dataMember) {
	jsonComponent[dataMemberName] = static_cast<float>(dataMember);
}
template<>
inline void SerializeProperty<ResourceID>(Json& jsonComponent, const char* dataMemberName, ResourceID const& dataMember) {
	jsonComponent[dataMemberName] = static_cast<size_t>(dataMember);
}
template<>
inline void SerializeProperty<std::unordered_map<MaterialName, Material>>(Json& jsonComponent, const char* dataMemberName, std::unordered_map<MaterialName, Material> const& dataMember) {
	std::vector<Json> jVec;

	//for each material in the map
	for (auto&& [materialName, material] : dataMember) {

		Json tempJson;

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
					Json tempJ;

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
			Json albedoJson;
			if constexpr (std::same_as<T, ResourceID>) {
				albedoJson["texture"] = static_cast<size_t>(albedo);
			}
			else /* its color */ {
				SerializeProperty<Color>(albedoJson, "color", albedo);
			}
			tempJson["albedo"] = albedoJson;
		}, material.albedo);

		jVec.push_back(tempJson);
	}
	jsonComponent[dataMemberName] = jVec;
}
template<>
inline void SerializeProperty<std::vector<ScriptData>> (Json& jsonComponent, const char* dataMemberName, std::vector<ScriptData> const& dataMember) {
	Json scriptArray;
	for (auto&& scriptData : dataMember) {
		Json scriptJson;
		Json fieldDataArray;

		for (auto&& field : scriptData.fields) {
			Json fieldDataJson;
			fieldDataJson["name"] = field.name;

			std::visit([&](auto&& data) {
				using Type = std::decay_t<decltype(data)>;
				SerializeProperty<Type>(fieldDataJson, "value", data);
				fieldDataJson["type"] = Family::id<Type>();
			}, field.data);

			fieldDataArray.push_back(std::move(fieldDataJson));
		}

		scriptJson["fields"] = std::move(fieldDataArray);
		scriptJson["id"] = static_cast<std::size_t>(scriptData.scriptId);
		scriptArray.push_back(std::move(scriptJson));
	}

	jsonComponent[dataMemberName] = std::move(scriptArray);
}
template<>
inline void SerializeProperty<std::unordered_map<std::string, AudioData>>(Json& jsonComponent, const char* dataMemberName, std::unordered_map<std::string, AudioData> const& dataMember) {
	Json audioArray;

	for (auto&& [name, audioData] : dataMember) {
		Json audioComponentJson;
		audioComponentJson["name"] = name;

		Json audioDataJson;
		audioDataJson["id"] = static_cast<std::size_t>(audioData.AudioId);
		audioDataJson["volume"] = audioData.Volume;

		audioComponentJson["audioData"] = audioDataJson;

		audioArray.push_back(std::move(audioComponentJson));
	}

	jsonComponent[dataMemberName] = std::move(audioArray);
}
template<>
inline void SerializeProperty<ParticleEmissionTypeSelection>(Json& jsonComponent, const char* dataMemberName, ParticleEmissionTypeSelection const& dataMember) {
	SerializeProperty<ParticleEmissionTypeSelection::EmissionShape>(jsonComponent[dataMemberName], "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			SerializeProperty<glm::vec3>(jsonComponent[dataMemberName], "Min", dataMember.cubeEmitter.min);
			SerializeProperty<glm::vec3>(jsonComponent[dataMemberName], "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			SerializeProperty<float>(jsonComponent[dataMemberName], "Radius", dataMember.radiusEmitter.radius);
			break;
		}
	}
}
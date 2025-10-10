#pragma once
#include "type_concepts.h"
#include "ECS/ECS.h"
#include "magic_enum.hpp"
#include "Logger.h"

#include <json/json.hpp>
#undef max
#undef min

using Json = nlohmann::json;
/***************************************************************************************
	Base
****************************************************************************************/
template<typename DataMemberType>
inline void DeserializeProperty(Json const& jsonComponent, const char* dataMemberName, DataMemberType& dataMember) {
	(void)jsonComponent, dataMemberName, dataMember;
};
/***************************************************************************************
	Excluded List, See type_concepts.h
****************************************************************************************/
template<NonSerializableTypes DataMemberType>
inline void DeserializeProperty(Json const& jsonComponent, const char* dataMemberName, DataMemberType& dataMember) {
	(void)jsonComponent, dataMemberName, dataMember;
};
/***************************************************************************************
	Special Constraints
****************************************************************************************/
template<IsTypedResourceID DataMemberType>
inline void DeserializeProperty(Json const& jsonComponent, const char* dataMemberName, DataMemberType& dataMember) {
	using OriginalAssetType = DataMemberType::AssetType;

	dataMember = TypedResourceID<OriginalAssetType>{ static_cast<std::size_t>(jsonComponent[dataMemberName]) };
}
template<IsFundamental DataMemberType>
inline void DeserializeProperty(Json const& jsonComponent, const char* dataMemberName, DataMemberType& dataMember) {
	dataMember = jsonComponent[dataMemberName];
}
template<IsEnum DataMemberType>
inline void DeserializeProperty(Json const& jsonComponent, const char* dataMemberName, DataMemberType& dataMember) {
	std::string str = jsonComponent[dataMemberName].dump();
	str = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));

	std::optional<DataMemberType> temp = magic_enum::enum_cast<DataMemberType>(str.c_str());

	if (temp.has_value()) {
		dataMember = temp.value();
	}
}
/***************************************************************************************
	Listings
****************************************************************************************/
template<>
inline void DeserializeProperty<entt::entity>(Json const& jsonComponent, const char* dataMemberName, entt::entity& dataMember) {
	try {
		unsigned int entityNum = jsonComponent[dataMemberName];
		entt::entity entity = entityNum == std::numeric_limits<unsigned int>::max() ? entt::null : static_cast<entt::entity>(entityNum);
		dataMember = entity;
	}
	catch (std::exception const& ex) {
		Logger::warn("Failed to parse entity data member. {}", ex.what());
		dataMember = entt::null;
	}
}
template<>
inline void DeserializeProperty<std::vector<entt::entity>>(Json const& jsonComponent, const char* dataMemberName, std::vector<entt::entity>& dataMember) {
	std::vector<entt::entity> vec;
	for (auto a : jsonComponent[dataMemberName])
		vec.push_back(static_cast<entt::entity>(a));
	dataMember = vec;
}
template<>
inline void DeserializeProperty<glm::vec2>(Json const& jsonComponent, const char* dataMemberName, glm::vec2& dataMember) {
	glm::vec2 vec{ jsonComponent[dataMemberName]["x"],
				   jsonComponent[dataMemberName]["y"] };
	dataMember = vec;
}
template<>
inline void DeserializeProperty<glm::vec3>(Json const& jsonComponent, const char* dataMemberName, glm::vec3& dataMember) {
	glm::vec3 vec{ jsonComponent[dataMemberName]["x"],
				   jsonComponent[dataMemberName]["y"],
				   jsonComponent[dataMemberName]["z"] };
	dataMember = vec;
}

template<>
inline void DeserializeProperty<Color>(Json const& jsonComponent, const char* dataMemberName, Color& dataMember) {
	glm::vec3 vec{ jsonComponent[dataMemberName]["r"],
				   jsonComponent[dataMemberName]["g"],
				   jsonComponent[dataMemberName]["b"] };
	dataMember = vec;
}
template<>
inline void DeserializeProperty<glm::quat>(Json const& jsonComponent, const char* dataMemberName, glm::quat& dataMember) {
	glm::quat vec{ jsonComponent[dataMemberName]["w"],
				   jsonComponent[dataMemberName]["x"],
				   jsonComponent[dataMemberName]["y"],
				   jsonComponent[dataMemberName]["z"] };
	dataMember = vec;
}
template<>
inline void DeserializeProperty<EulerAngles>(Json const& jsonComponent, const char* dataMemberName, EulerAngles& dataMember) {
	glm::vec3 vec{ jsonComponent[dataMemberName]["x"],
				   jsonComponent[dataMemberName]["y"],
				   jsonComponent[dataMemberName]["z"] };
	dataMember = vec;
}
template<>
inline void DeserializeProperty<Radian>(Json const& jsonComponent, const char* dataMemberName, Radian& dataMember) {
	float angle = jsonComponent[dataMemberName];
	dataMember = Radian{ angle };
}
template<>
inline void DeserializeProperty<Degree>(Json const& jsonComponent, const char* dataMemberName, Degree& dataMember) {
	float angle = jsonComponent[dataMemberName];
	dataMember = Degree{ angle };
}
template<>
inline void DeserializeProperty<ResourceID>(Json const& jsonComponent, const char* dataMemberName, ResourceID& dataMember) {
	dataMember = static_cast<ResourceID>(jsonComponent[dataMemberName]);
}
template<>
inline void DeserializeProperty<std::unordered_map<MaterialName, Material>>(Json const& jsonComponent, const char* dataMemberName, std::unordered_map<MaterialName, Material>& dataMember) {
	(void)dataMemberName;
	std::unordered_map<MaterialName, Material> map;
	//material and model id
	for (auto json : jsonComponent["materials"]) {
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

				config.roughness = json["config"]["roughness"];
				config.metallic = json["config"]["metallic"];
				config.occulusion = json["config"]["occulusion"];

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
template<>
inline void DeserializeProperty<std::vector<ScriptData>>(Json const& jsonComponent, const char* dataMemberName, std::vector<ScriptData>& dataMember) {
	(void)dataMemberName;
	std::vector<ScriptData> allScripts;

	// parsing every script...
	for (auto&& scriptJson : jsonComponent["scriptDatas"]) {
		// for each script, they have an id and a field..
		std::vector<FieldData> fields;

		// let's parse all fields of a given script..
		for (auto&& fieldJson : scriptJson["fields"]) {
			FieldData scriptFieldData;
			scriptFieldData.name = fieldJson["name"];
			auto parseVariantJson = [&]<typename ...Types>() {
				([&]() {
					if (Family::id<Types>() == fieldJson["type"]) {
						Types temp;
						DeserializeProperty<Types>(fieldJson, "value", temp);
						scriptFieldData.data = temp;
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
template<>
inline void DeserializeProperty<std::unordered_map<std::string, AudioData>>(Json const& jsonComponent, const char* dataMemberName, std::unordered_map<std::string, AudioData>& dataMember) {
	for (auto&& audioComponent : jsonComponent[dataMemberName]) {
		std::string name = audioComponent["name"];

		std::size_t id = audioComponent["audioData"]["id"];
		float volume = audioComponent["audioData"]["volume"];

		dataMember.insert({ std::move(name), AudioData{ id, volume } });
	}
}
template<>
inline void DeserializeProperty<ParticleEmissionTypeSelection>(Json const& jsonComponent, const char* dataMemberName, ParticleEmissionTypeSelection& dataMember) {
	DeserializeProperty<ParticleEmissionTypeSelection::EmissionShape>(jsonComponent[dataMemberName], "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			DeserializeProperty<glm::vec3>(jsonComponent[dataMemberName], "Min", dataMember.cubeEmitter.min);
			DeserializeProperty<glm::vec3>(jsonComponent[dataMemberName], "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
			DeserializeProperty<float>(jsonComponent[dataMemberName], "Radius", dataMember.sphereEmitter.radius);
			break;
		}
	}
}
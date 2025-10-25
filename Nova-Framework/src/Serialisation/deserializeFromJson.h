#pragma once
#include "type_concepts.h"
#include "Component.h"
#include "magic_enum.hpp"
#include "Logger.h"

#include <json/json.hpp>
#undef max
#undef min

using Json = nlohmann::json;
/***************************************************************************************
	Base
****************************************************************************************/
template<typename T>
inline void deserializeFromJson(T& dataMember, Json const& json) {
	[] <bool flag = false, typename Ty = T, bool test = reflection::conceptReflectable<T>>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
	}();
};

/***************************************************************************************
	Excluded List, See type_concepts.h
****************************************************************************************/
template<NonSerializableTypes DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	(void)json, dataMember;
};

/***************************************************************************************
	Special Constraints
****************************************************************************************/
template<IsTypedResourceID DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	using OriginalAssetType = DataMemberType::AssetType;

	dataMember = TypedResourceID<OriginalAssetType>{ static_cast<std::size_t>(json) };
}

template<IsFundamental DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	dataMember = json;
}

template<IsEnum DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	std::string str = json.dump();
	str = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));

	std::optional<DataMemberType> temp = magic_enum::enum_cast<DataMemberType>(str);

	if (temp.has_value()) {
		dataMember = temp.value();
	}
	else {
		Logger::error("Failed to deserialise enum.");
	}
}

// ========================== variant.. ========================================= 
template <typename... Ts>
void initialiseVariant(Json const& json, std::size_t i, std::variant<Ts...>& variant) {
	assert(i < sizeof...(Ts));
	static std::variant<Ts...> table[] = { Ts{ }... };

	// we now retrieve the variant with the type belonging to this specific index.
	variant = table[i];

	// we then initialise the underlying variant value..
	std::visit([&](auto&& value) {
		deserializeFromJson(value, json);
	}, variant);
}

template <typename DataMemberType> requires is_variant<DataMemberType>::value
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	if (json.contains("__variant_type") && json.contains("__variant_value")) {
		std::size_t index = json["__variant_type"];
		initialiseVariant(json["__variant_value"], index, dataMember);
	}
}
// ============================================================================== 

// optional..
template <isOptional DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	if (!json.is_null()) {
		deserializeFromJson(dataMember, json);
	}
}

/***************************************************************************************
	Containers..
****************************************************************************************/
// vectors
template <isVector DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	dataMember.clear();

	for (Json const& elementJson : json) {
		typename DataMemberType::value_type elementType;
		deserializeFromJson(elementType, elementJson);
		dataMember.push_back(elementType);
	}
}

// unordered_map
template <isUnorderedMap DataMemberType>
inline void deserializeFromJson(DataMemberType& dataMember, Json const& json) {
	dataMember.clear();

	for (auto&& [name, elementJson] : json.items()) {
		typename DataMemberType::key_type key = static_cast<typename DataMemberType::key_type>(name);
		typename DataMemberType::mapped_type value;

		deserializeFromJson(value, elementJson);

		std::pair<const typename DataMemberType::key_type, typename DataMemberType::mapped_type> pair{
			std::move(key), std::move(value)
		};

		dataMember.insert(std::move(pair));
	}
}

/***************************************************************************************
	Recursive reflection, also entry point.
****************************************************************************************/
template <reflection::conceptReflectable T>
inline void deserializeFromJson(T& dataMember, Json const& json) {
	reflection::visit([&](auto fieldData) {
		auto& dataMember = fieldData.get();
		constexpr const char* dataMemberName = fieldData.name();
		using DataMemberType = std::decay_t<decltype(dataMember)>;

		deserializeFromJson(dataMember, json[dataMemberName]);
	}, dataMember);
}

/***************************************************************************************
	Listings
****************************************************************************************/
template<>
inline void deserializeFromJson<NormalizedFloat>(NormalizedFloat& dataMember, Json const& json) {
	float value = json;
	dataMember = value;
}

template<>
inline void deserializeFromJson<entt::entity>(entt::entity& dataMember, Json const& json) {
	unsigned int entityNum = json;
	entt::entity entity = entityNum == std::numeric_limits<unsigned int>::max() ? entt::null : static_cast<entt::entity>(entityNum);
	dataMember = entity;
}

template<>
inline void deserializeFromJson<glm::vec2>(glm::vec2& dataMember, Json const& json) {
	glm::vec2 vec{ json["x"], json["y"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<glm::vec3>(glm::vec3& dataMember, Json const& json) {
	glm::vec3 vec{ json["x"], json["y"], json["z"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<glm::vec4>(glm::vec4& dataMember, Json const& json) {
	glm::vec4 vec{ json["x"], json["y"], json["z"], json["w"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<Color>(Color& dataMember, Json const& json) {
	glm::vec3 vec{ json["r"], json["g"], json["b"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<ColorA>(ColorA& dataMember, Json const& json) {
	glm::vec4 vec{ json["r"], json["g"], json["b"], json["a"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<glm::quat>(glm::quat& dataMember, Json const& json) {
	glm::quat vec{ json["w"], json["x"], json["y"], json["z"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<glm::mat4x4>(glm::mat4x4& dataMember, Json const& json) {
	float* floatArray = glm::value_ptr(dataMember);

	for (int i = 0; i < 4 * 4; ++i) {
		floatArray[i] = json[i];
	}
}

template<>
inline void deserializeFromJson<glm::mat3x3>(glm::mat3x3& dataMember, Json const& json) {
	float* floatArray = glm::value_ptr(dataMember);

	for (int i = 0; i < 3 * 3; ++i) {
		floatArray[i] = json[i];
	}
}

template<>
inline void deserializeFromJson<EulerAngles>(EulerAngles& dataMember, Json const& json) {
	glm::vec3 vec{ json["x"], json["y"], json["z"] };
	dataMember = vec;
}

template<>
inline void deserializeFromJson<Radian>(Radian& dataMember, Json const& json) {
	float angle = json;
	dataMember = Radian{ angle };
}

template<>
inline void deserializeFromJson<Degree>(Degree& dataMember, Json const& json) {
	float angle = json;
	dataMember = Degree{ angle };
}

template<>
inline void deserializeFromJson<ResourceID>(ResourceID& dataMember, Json const& json) {
	dataMember = static_cast<ResourceID>(static_cast<std::size_t>(json));
}

template<>
inline void deserializeFromJson<ControllerNodeID>(ControllerNodeID& dataMember, Json const& json) {
	dataMember = static_cast<ControllerNodeID>(static_cast<std::size_t>(json));
}

#if 0
template<>
inline void deserializeFromJson(std::unordered_map<MaterialName, Material>& dataMember) {
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
inline void deserializeFromJson(std::vector<ScriptData>& dataMember) {
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
						deserializeFromJson<Types>(fieldJson, "value", temp);
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
inline void deserializeFromJson(std::unordered_map<std::string, AudioData>& dataMember) {
	for (auto&& audioComponent : jsonComponent[dataMemberName]) {
		std::string name = audioComponent["name"];

		std::size_t id = audioComponent["audioData"]["id"];
		float volume = audioComponent["audioData"]["volume"];

		dataMember.insert({ std::move(name), AudioData{ id, volume } });
	}
}
template<>
inline void deserializeFromJson(ParticleEmissionTypeSelection& dataMember) {
	deserializeFromJson<ParticleEmissionTypeSelection::EmissionShape>(jsonComponent[dataMemberName], "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			deserializeFromJson<glm::vec3>(jsonComponent[dataMemberName], "Min", dataMember.cubeEmitter.min);
			deserializeFromJson<glm::vec3>(jsonComponent[dataMemberName], "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
			deserializeFromJson<float>(jsonComponent[dataMemberName], "Arc", dataMember.coneEmitter.arc);
			deserializeFromJson<float>(jsonComponent[dataMemberName], "Distance", dataMember.coneEmitter.distance);
			deserializeFromJson<float>(jsonComponent[dataMemberName], "Radius", dataMember.radiusEmitter.radius);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			deserializeFromJson<float>(jsonComponent[dataMemberName], "Radius", dataMember.radiusEmitter.radius);
			break;
		}
	}
}
template<>
inline void deserializeFromJson(ParticleColorSelection& dataMember) {
	deserializeFromJson<bool>(jsonComponent[dataMemberName], "Randomized Color", dataMember.randomizedColor);
	if (!dataMember.randomizedColor)
		deserializeFromJson<ColorA>(jsonComponent[dataMemberName], "Color", dataMember.color);
}
template<>
inline void deserializeFromJson(SizeOverLifetime& dataMember) {
	deserializeFromJson<bool>(jsonComponent[dataMemberName], "Selected", dataMember.selected);
	if (dataMember.selected) {
		deserializeFromJson<InterpolationType>(jsonComponent[dataMemberName], "InterpolationType", dataMember.interpolationType);
		deserializeFromJson<float>(jsonComponent[dataMemberName], "EndSize", dataMember.endSize);
	}
}
template<>
inline void deserializeFromJson(ColorOverLifetime& dataMember) {
	deserializeFromJson<bool>(jsonComponent[dataMemberName], "Selected", dataMember.selected);
	if (dataMember.selected) {
		deserializeFromJson<InterpolationType>(jsonComponent[dataMemberName], "InterpolationType", dataMember.interpolationType);
		deserializeFromJson<ColorA>(jsonComponent[dataMemberName], "EndColor", dataMember.endColor);
	}
}
#endif
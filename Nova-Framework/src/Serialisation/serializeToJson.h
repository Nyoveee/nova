#pragma once
#include "type_concepts.h"
#include "Component.h"
#include "magic_enum.hpp"

#include <json/json.hpp>

using Json = nlohmann::json;

/***************************************************************************************
	Base case.
****************************************************************************************/
template<typename T>
inline Json serializeToJson(T const& dataMember);

/***************************************************************************************
	Explicit exclusion list, See type_concepts.h
****************************************************************************************/
template<NonSerializableTypes DataMemberType>
inline Json serializeToJson([[maybe_unused]] DataMemberType const& dataMember) {
	return {};
}

/***************************************************************************************
	Special Constraints
****************************************************************************************/
// Typed IDs..
template<IsTypedResourceID DataMemberType>
inline Json serializeToJson(DataMemberType const& dataMember) {
	return static_cast<size_t>(dataMember);
}

// Fundamentals + std::string
template<IsFundamental DataMemberType>
inline Json serializeToJson(DataMemberType const& dataMember) {
	return dataMember;
}

// enum..
template<IsEnum DataMemberType>
inline Json serializeToJson(DataMemberType const& dataMember) {
	return magic_enum::enum_name(dataMember);
}

// variant..
template <typename DataMemberType> requires is_variant<DataMemberType>::value
inline Json serializeToJson(DataMemberType const& variant) {
	Json json;

	std::visit([&](auto&& value) {
		json["__variant_value"] = serializeToJson(value);
	}, variant);
	
	json["__variant_type"] = variant.index();

	return json;
}

// optional..
template <isOptional DataMemberType>
inline Json serializeToJson(DataMemberType const& optional) {
	if (optional) {
		return serializeToJson(optional.value());
	}
	else {
		return Json{};
	}
}

/***************************************************************************************
	Containers..
****************************************************************************************/
// vectors
template <isVector DataMemberType>
inline Json serializeToJson(DataMemberType const& dataMember) {
	Json jsonArray;

	for (auto&& element : dataMember) {
		jsonArray.push_back(serializeToJson(element));
	}

	return jsonArray;
}

// unordered_map
template <isUnorderedMap DataMemberType>
inline Json serializeToJson(DataMemberType const& dataMember) {
	Json json;

	for (auto&& [key, value] : dataMember) {
		json[static_cast<std::string>(key)] = serializeToJson(value);
	}

	return json;
}

/***************************************************************************************
	Recursive reflection, also entry point.
****************************************************************************************/
template <reflection::conceptReflectable T>
inline Json serializeToJson(T const& dataMember) {
	Json json;

	reflection::visit([&](auto fieldData) {
		auto& dataMember = fieldData.get();
		constexpr const char* dataMemberName = fieldData.name();
		using DataMemberType = std::decay_t<decltype(dataMember)>;

		json[dataMemberName] = serializeToJson<DataMemberType>(dataMember);
	}, dataMember);

	return json;
}

/***************************************************************************************
	Explicit specialization - Listings
****************************************************************************************/
template<>
inline Json serializeToJson<glm::vec2>(glm::vec2 const& dataMember) {
	Json json;

	json["x"] = dataMember.x;
	json["y"] = dataMember.y;

	return json;
}

template<>
inline Json serializeToJson<glm::vec3>(glm::vec3 const& dataMember) {
	Json json;

	json["x"] = dataMember.x;
	json["y"] = dataMember.y;
	json["z"] = dataMember.z;

	return json;
}

template<>
inline Json serializeToJson<entt::entity>(entt::entity const& dataMember) {
	return static_cast<unsigned int>(dataMember);
}

#if 0
template<>
inline Json serializeToJson<std::vector<entt::entity>>(std::vector<entt::entity> const& dataMember) {
	jsonComponent[dataMemberName] = dataMember;
}
#endif

template<>
inline Json serializeToJson<Color>(Color const& dataMember) {
	Json json;

	json["r"] = dataMember.r();
	json["g"] = dataMember.g();
	json["b"] = dataMember.b();

	return json;
}

template<>
inline Json serializeToJson<ColorA>(ColorA const& dataMember) {
	Json json;

	json["r"] = dataMember.r();
	json["g"] = dataMember.g();
	json["b"] = dataMember.b();
	json["a"] = dataMember.a();
	
	return json;
}

template<>
inline Json serializeToJson<glm::quat>(glm::quat const& dataMember) {
	Json json;

	json["w"] = dataMember.w;
	json["x"] = dataMember.x;
	json["y"] = dataMember.y;
	json["z"] = dataMember.z;

	return json;
}

template<>
inline Json serializeToJson<EulerAngles>(EulerAngles const& dataMember) {
	Json json;

	json["x"] = dataMember.angles.x;
	json["y"] = dataMember.angles.y;
	json["z"] = dataMember.angles.z;

	return json;
}

template<>
inline Json serializeToJson<Radian>(Radian const& dataMember) {
	return static_cast<float>(dataMember);
}

template<>
inline Json serializeToJson<Degree>(Degree const& dataMember) {
	return static_cast<float>(dataMember);
}

template<>
inline Json serializeToJson<ResourceID>(ResourceID const& dataMember) {
	return static_cast<size_t>(dataMember);
}

template<>
inline Json serializeToJson<ControllerNodeID>(ControllerNodeID const& controllerId) {
	return static_cast<size_t>(controllerId);
}

#if 0
template<>
inline Json serializeToJson<std::unordered_map<MaterialName, Material>>(std::unordered_map<MaterialName, Material> const& dataMember) {
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
				serializeToJson<Color>(albedoJson, "color", albedo);
			}
			tempJson["albedo"] = albedoJson;
			}, material.albedo);

		jVec.push_back(tempJson);
	}
	jsonComponent[dataMemberName] = jVec;
}

template<>
inline Json serializeToJson<std::vector<ScriptData>>(std::vector<ScriptData> const& dataMember) {
	Json scriptArray;
	for (auto&& scriptData : dataMember) {
		Json scriptJson;
		Json fieldDataArray;

		for (auto&& field : scriptData.fields) {
			Json fieldDataJson;
			fieldDataJson["name"] = field.name;

			std::visit([&](auto&& data) {
				using Type = std::decay_t<decltype(data)>;
				serializeToJson<Type>(fieldDataJson, "value", data);
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
inline Json serializeToJson<std::unordered_map<std::string, AudioData>>(std::unordered_map<std::string, AudioData> const& dataMember) {
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
inline Json serializeToJson(ParticleEmissionTypeSelection const& dataMember) {
	serializeToJson<ParticleEmissionTypeSelection::EmissionShape>(jsonComponent[dataMemberName], "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			serializeToJson<glm::vec3>(jsonComponent[dataMemberName], "Min", dataMember.cubeEmitter.min);
			serializeToJson<glm::vec3>(jsonComponent[dataMemberName], "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
			serializeToJson<float>(jsonComponent[dataMemberName], "Arc", dataMember.coneEmitter.arc);
			serializeToJson<float>(jsonComponent[dataMemberName], "Distance", dataMember.coneEmitter.distance);
			serializeToJson<float>(jsonComponent[dataMemberName], "Radius", dataMember.radiusEmitter.radius);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			serializeToJson<float>(jsonComponent[dataMemberName], "Radius", dataMember.radiusEmitter.radius);
			break;
		}
	}
}

template<>
inline Json serializeToJson(ParticleColorSelection const& dataMember) {
	serializeToJson<bool>(jsonComponent[dataMemberName], "Randomized Color", dataMember.randomizedColor);
	if (!dataMember.randomizedColor)
		serializeToJson<ColorA>(jsonComponent[dataMemberName], "Color", dataMember.color);
}

template<>
inline Json serializeToJson(SizeOverLifetime const& dataMember) {
	serializeToJson<bool>(jsonComponent[dataMemberName], "Selected", dataMember.selected);
	if (dataMember.selected) {
		serializeToJson<InterpolationType>(jsonComponent[dataMemberName], "InterpolationType", dataMember.interpolationType);
		serializeToJson<float>(jsonComponent[dataMemberName], "EndSize", dataMember.endSize);
	}
}

template<>
inline Json serializeToJson(ColorOverLifetime const& dataMember) {
	serializeToJson<bool>(jsonComponent[dataMemberName], "Selected", dataMember.selected);
	if (dataMember.selected) {
		serializeToJson<InterpolationType>(jsonComponent[dataMemberName], "InterpolationType", dataMember.interpolationType);
		serializeToJson<ColorA>(jsonComponent[dataMemberName], "EndColor", dataMember.endColor);
	}
}
#endif

/***************************************************************************************
	Base
****************************************************************************************/
// this function variant attempts to retrieve data member 
template<typename T>
inline Json serializeToJson(T const& dataMember) {
	[] <bool flag = false, typename Ty = T, bool test = IsFundamental<T>>() {
		static_assert(flag, "Did not account for all data members. " __FUNCSIG__);
	}();
}
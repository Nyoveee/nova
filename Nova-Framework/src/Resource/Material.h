#pragma once

#include "resource.h"
#include "systemResource.h"
#include <glm/glm.hpp>
#include "reflection.h"

class CustomShader;

// ================================================================================================================================
class Texture;

#define AllUniformTypes \
	bool, int, unsigned int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, TypedResourceID<Texture>, Color, ColorA, NormalizedFloat

struct OverriddenUniformData {
	std::string type;
	std::variant<AllUniformTypes> value;

	REFLECTABLE(
		type,
		value
	)
};

class CustomShader;

struct MaterialData {
	TypedResourceID<CustomShader> selectedShader								{ DEFAULT_PBR_SHADER_ID };
	std::unordered_map<std::string, OverriddenUniformData> overridenUniforms	{ 
		//{ "colorTint",	OverriddenUniformData{ "Color",				Color{ 1.f, 1.f, 1.f } }						},
		//{ "albedoMap",	OverriddenUniformData{ "sampler2D",			TypedResourceID<Texture>{ NONE_TEXTURE_ID } }	},
		//{ "roughness",	OverriddenUniformData{ "NormalizedFloat",	NormalizedFloat{ 0.28f } }						},
		//{ "metallic",	OverriddenUniformData{ "NormalizedFloat",	NormalizedFloat{} }								},
		//{ "occulusion",	OverriddenUniformData{ "NormalizedFloat",	NormalizedFloat{ 1.f} }							}
	};

	REFLECTABLE(
		selectedShader,
		overridenUniforms
	)
};
// ================================================================================================================================

class Material : public Resource {
public:
	FRAMEWORK_DLL_API Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData materialData);
	FRAMEWORK_DLL_API ~Material();

	FRAMEWORK_DLL_API Material(Material const& other) = delete;
	FRAMEWORK_DLL_API Material(Material&& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material const& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material&& other) = delete;

public:
	MaterialData materialData;
};
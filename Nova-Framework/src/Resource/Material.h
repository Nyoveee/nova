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
	std::unordered_map<std::string, OverriddenUniformData> overridenUniforms	{ };

	REFLECTABLE(
		selectedShader,
		overridenUniforms
	)
};
// ================================================================================================================================

class Material : public Resource {
public:
	FRAMEWORK_DLL_API Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData materialData);

public:
	MaterialData materialData;
};
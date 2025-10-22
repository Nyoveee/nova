#pragma once
#include "resource.h"
#include "shader.h"
#include <glm/glm.hpp>
#include "reflection.h"
#define AllUniformTypes \
	bool, int, unsigned int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3,glm::mat4

class Material : public Resource {
public:
	enum class Pipeline {
		PBR,			// uses everything.
		Color,			// only uses albedo.
	};
	struct OverriddenUniformData {
		std::string type;
		std::variant<AllUniformTypes> value;
	};
	struct MaterialData {
		TypedResourceID<CustomShader> selectedShader;
		Pipeline selectedPipeline = Pipeline::PBR;
		std::unordered_map<std::string, OverriddenUniformData> overridenUniforms;
		REFLECTABLE(
			selectedShader,
			selectedPipeline,
			overridenUniforms,
		)
	} materialData;
public:
	FRAMEWORK_DLL_API Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData const& materialData);
	FRAMEWORK_DLL_API ~Material();

	FRAMEWORK_DLL_API Material(Material const& other) = delete;
	FRAMEWORK_DLL_API Material(Material&& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material const& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material&& other) = delete;
};
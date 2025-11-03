#pragma once

#include "resource.h"
#include "export.h"
#include "shader.h"

#include <optional>
#include <glm/glm.hpp>
#include <unordered_map>

using ShaderVariableName = std::string;
using ShaderVariableType = std::string;

class ResourceManager;

enum class Pipeline {
	PBR,			// uses everything.
	Color,			// only uses albedo.
};

class CustomShader: public Resource
{
public:
	enum class BlendingConfig {
		AlphaBlending,
		AdditiveBlending,
		PureAdditiveBlending,
		PremultipliedAlpha,
		Disabled
	};

	enum class DepthTestingMethod {
		DepthTest,
		NoDepthWrite,
		NoDepthWriteTest
	};

	enum class CullingConfig {
		Enable,
		Disable
	};

	// this struct will be de/serialisation for the construction of this resource custom shader	.
	struct ShaderParserData {
		// Tags(Defaulted)
		BlendingConfig blendingConfig = CustomShader::BlendingConfig::AdditiveBlending;
		DepthTestingMethod depthTestingMethod = CustomShader::DepthTestingMethod::DepthTest;
		CullingConfig cullingConfig = CustomShader::CullingConfig::Enable;

		// Properties(name,type)
		std::unordered_map<ShaderVariableName, ShaderVariableType> uniforms;
		
		// Code
		std::string vShaderCode;
		std::string fShaderCode;

		Pipeline pipeline;

		REFLECTABLE(
			blendingConfig,
			depthTestingMethod,
			cullingConfig,
			uniforms,
			vShaderCode,
			fShaderCode,
			pipeline
		)

	} customShaderData;

public:
	FRAMEWORK_DLL_API CustomShader(ResourceID id, ResourceFilePath resourceFilePath, ShaderParserData shaderData);
	FRAMEWORK_DLL_API ~CustomShader();

	FRAMEWORK_DLL_API CustomShader(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader(CustomShader&& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader&& other) = delete;

public:
	FRAMEWORK_DLL_API void compile();
	FRAMEWORK_DLL_API std::optional<Shader> const& getShader() const;

private:
	std::optional<Shader> shader;

public:
	static inline const std::unordered_set<std::string> validGlslPrimitive {
		"bool", "int", "uint", "float", "vec2", "vec3", "vec4", "mat3", "mat4", "sampler2D"
	};

	static inline const std::unordered_set<std::string> validCustomTypes{
		"Color", "ColorA", "NormalizedFloat"
	};

	static inline const std::unordered_map<std::string, std::string> customTypeToGlslPrimitive{
		{ "Color",			"vec3"		},
		{ "ColorA",			"vec4"		},
		{ "NormalizedFloat", "float"	},
	};

	static inline const std::unordered_set<std::string> allValidShaderTypes = {
		"bool", "int", "uint", "float", "vec2", "vec3", "vec4", "mat3", "mat4", "sampler2D",
		"Color", "ColorA", "NormalizedFloat"
	};
};

template <>
struct AssetInfo<CustomShader> : public BasicAssetInfo {
	Pipeline pipeline;
};
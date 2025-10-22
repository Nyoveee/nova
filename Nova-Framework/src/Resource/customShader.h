#pragma once

#include "resource.h"
#include "shader.h"
#include "export.h"
#include "Material.h"

#include <glm/glm.hpp>
#include <unordered_map>

using ShaderVariableName = std::string;
using ShaderVariableType = std::string;

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

	// this struct will be de/serialisation for the construction of this resource custom shader	.
	struct ShaderParserData {
		// Tags(Defaulted)
		BlendingConfig blendingConfig = CustomShader::BlendingConfig::AdditiveBlending;
		DepthTestingMethod depthTestingMethod = CustomShader::DepthTestingMethod::DepthTest;
		
		// Properties(name,type)
		std::unordered_map<ShaderVariableName, ShaderVariableType> uniforms;
		
		// Code
		std::string fShaderCode;

		REFLECTABLE(
			blendingConfig,
			depthTestingMethod,
			uniforms,
			fShaderCode
		)
	}customShaderData;
public:
	FRAMEWORK_DLL_API CustomShader(ResourceID id, ResourceFilePath resourceFilePath, ShaderParserData shaderData);
	FRAMEWORK_DLL_API ~CustomShader();

	FRAMEWORK_DLL_API CustomShader(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader(CustomShader&& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader&& other) = delete;
public:
	FRAMEWORK_DLL_API void use(Material::MaterialData const& data);
	FRAMEWORK_DLL_API void compileWithPipeline(Material const& material);
private:
	std::unordered_map<Material::Pipeline, std::optional<Shader>> shaders;
};


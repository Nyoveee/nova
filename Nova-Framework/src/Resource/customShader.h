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

		// Pipeline (dont need to include this in parser..)
		Pipeline pipeline = Pipeline::PBR;

		REFLECTABLE(
			blendingConfig,
			depthTestingMethod,
			uniforms,
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
	FRAMEWORK_DLL_API Pipeline getPipeline() const;

private:
	std::optional<Shader> shader;
};


#pragma once

#include "resource.h"
#include "export.h"
#include "shader.h"

#include "materialConfig.h"

#include <optional>
#include <glm/glm.hpp>
#include <unordered_map>

class ResourceManager;

enum class Pipeline {
	PBR,			// uses everything.
	Color,			// only uses albedo.
};

class CustomShader: public Resource
{
public:
	// this struct will be de/serialisation for the construction of this resource custom shader	.
	struct ShaderParserData {
		// Tags (Defaulted)
		BlendingConfig blendingConfig			= BlendingConfig::AdditiveBlending;
		DepthTestingMethod depthTestingMethod	= DepthTestingMethod::DepthTest;
		CullingConfig cullingConfig				= CullingConfig::Enable;

		std::vector<UniformData> uniformDatas	{};
		
		// Code
		std::string vShaderCode;
		std::string fShaderCode;

		Pipeline pipeline;

		REFLECTABLE(
			blendingConfig,
			depthTestingMethod,
			cullingConfig,
			uniformDatas,
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

};

template <>
struct AssetInfo<CustomShader> : public BasicAssetInfo {
	Pipeline pipeline;
};
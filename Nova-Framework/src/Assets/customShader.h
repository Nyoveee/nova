#pragma once
#include "resource.h"
#include "export.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>

using ShaderVariableName = std::string;
using ShaderVariableType = std::string;

class CustomShader: public Resource
{
public:
	enum class Pipeline {
		PBR,			// uses everything.
		Color,			// only uses albedo.
		Failed
	};
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
		std::unordered_map<std::string, std::string> uniforms;
		
		// Code
		std::string fShaderCode;

		REFLECTABLE(
			blendingConfig,
			depthTestingMethod,
			uniforms,
			fShaderCode
		)
	};

public:
	FRAMEWORK_DLL_API CustomShader(ResourceID id, ResourceFilePath resourceFilePath, ShaderParserData shaderData);
	FRAMEWORK_DLL_API ~CustomShader();

	FRAMEWORK_DLL_API CustomShader(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader(CustomShader&& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader const& other) = delete;
	FRAMEWORK_DLL_API CustomShader& operator=(CustomShader&& other) = delete;

public:
	// setting uniform functions
	FRAMEWORK_DLL_API void setBool(const std::string& name, bool value) const;
	FRAMEWORK_DLL_API void setInt(const std::string& name, int value) const;
	FRAMEWORK_DLL_API void setUInt(const std::string& name, unsigned int value) const;
	FRAMEWORK_DLL_API void setFloat(const std::string& name, float value) const;
	FRAMEWORK_DLL_API void setVec2(const std::string& name, float x, float y) const;
	FRAMEWORK_DLL_API void setVec2(const std::string& name, glm::vec2 const& list) const;
	FRAMEWORK_DLL_API void setVec3(const std::string& name, glm::vec3 const& list) const;
	FRAMEWORK_DLL_API void setVec3(const std::string& name, float x, float y, float z) const;
	FRAMEWORK_DLL_API void setVec4(const std::string& name, glm::vec4 const& list) const;
	FRAMEWORK_DLL_API void setMatrix(const std::string& name, const glm::mat4x4& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setMatrix(const std::string& name, const glm::mat3x3& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setImageUniform(const std::string& name, int uniform) const;

public:
	// accessor
	FRAMEWORK_DLL_API GLuint id() const;
	// use/activate the shader
	FRAMEWORK_DLL_API void use(Pipeline pipeline) const;
	FRAMEWORK_DLL_API void unuse() const;

private:
	GLuint m_id; // To Do, a vector of ids to store compatible shaders
	BlendingConfig blendingConfig;
	DepthTestingMethod depthTestingMethod;
	std::unordered_map<ShaderVariableName, ShaderVariableType> uniforms;
};


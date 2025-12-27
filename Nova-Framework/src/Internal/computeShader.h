#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <glm/glm.hpp>

#include "export.h"

using GLuint = unsigned int;

class ComputeShader
{
public:
	enum class ShaderCompileStatus {
		Success,
		Failed
	} shaderCompileStatus;

public:
	FRAMEWORK_DLL_API ComputeShader(const char* computeShaderPath);
	FRAMEWORK_DLL_API ~ComputeShader();

	ComputeShader(ComputeShader const& other) = delete;
	ComputeShader(ComputeShader&& other) noexcept;
	ComputeShader& operator=(ComputeShader const& other) = delete;
	ComputeShader& operator=(ComputeShader&& other) noexcept;

public:
	// accessor
	FRAMEWORK_DLL_API GLuint id() const;

	// use/activate the shader
	FRAMEWORK_DLL_API void use() const;
	FRAMEWORK_DLL_API void unuse() const;

	// getters
	FRAMEWORK_DLL_API std::string const& getErrorMessage() const;

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
	FRAMEWORK_DLL_API void compile();

private:
	GLuint m_id;
	std::string computeShaderCode;

	std::string errorMessage;

	const char* computeShaderPath = nullptr;
};

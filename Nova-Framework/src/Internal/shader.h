#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <variant>
#include <unordered_map>
#include <glm/glm.hpp>

#include "type_alias.h"
#include "export.h"
#include "reflection.h"

using GLuint = unsigned int;
using GLint = int;

class Shader
{
public:
	enum class ShaderCompileStatus {
		Success,
		Failed
	} shaderCompileStatus;

public:
	FRAMEWORK_DLL_API Shader(const char* vertexPath, const char* fragmentPath);
	FRAMEWORK_DLL_API Shader(std::string vertexCode, std::string fragmentCode);
	FRAMEWORK_DLL_API ~Shader();

	Shader(Shader const& other) = delete;
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader const& other) = delete;
	Shader& operator=(Shader&& other) noexcept;

public:
	// accessor
	FRAMEWORK_DLL_API GLuint id() const;

	// use/activate the shader
	FRAMEWORK_DLL_API void use() const;
	FRAMEWORK_DLL_API void unuse() const;

	// getters
	FRAMEWORK_DLL_API std::string const& getVertexShader() const;
	FRAMEWORK_DLL_API std::string const& getFragmentShader() const;
	FRAMEWORK_DLL_API std::string const& getErrorMessage() const;
	FRAMEWORK_DLL_API bool				 hasCompiled() const;

public:
	// setting uniform functions
	FRAMEWORK_DLL_API void setBool			(const char* name, bool value) const;
	FRAMEWORK_DLL_API void setInt			(const char* name, int value) const;
	FRAMEWORK_DLL_API void setUInt			(const char* name, unsigned int value) const;
	FRAMEWORK_DLL_API void setFloat			(const char* name, float value) const;
	FRAMEWORK_DLL_API void setVec2			(const char* name, float x, float y) const;
	FRAMEWORK_DLL_API void setVec2			(const char* name, glm::vec2 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(const char* name, glm::vec3 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(const char* name, float x, float y, float z) const;
	FRAMEWORK_DLL_API void setVec4			(const char* name, glm::vec4 const& list) const;
	FRAMEWORK_DLL_API void setUVec2			(const char* name, glm::uvec2 const& list) const;
	FRAMEWORK_DLL_API void setUVec3			(const char* name, glm::uvec3 const& list) const;
	FRAMEWORK_DLL_API void setMatrix		(const char* name, const glm::mat4x4& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setMatrix		(const char* name, const glm::mat3x3& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setImageUniform	(const char* name, int textureUnit) const;

	FRAMEWORK_DLL_API GLint getUniformLocation(const char* name) const;

	// we set value if we have definite location..
	FRAMEWORK_DLL_API void setBool			(GLint location, bool value) const;
	FRAMEWORK_DLL_API void setInt			(GLint location, int value) const;
	FRAMEWORK_DLL_API void setUInt			(GLint location, unsigned int value) const;
	FRAMEWORK_DLL_API void setFloat			(GLint location, float value) const;
	FRAMEWORK_DLL_API void setVec2			(GLint location, float x, float y) const;
	FRAMEWORK_DLL_API void setVec2			(GLint location, glm::vec2 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(GLint location, glm::vec3 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(GLint location, float x, float y, float z) const;
	FRAMEWORK_DLL_API void setVec4			(GLint location, glm::vec4 const& list) const;
	FRAMEWORK_DLL_API void setUVec2			(GLint location, glm::uvec2 const& list) const;
	FRAMEWORK_DLL_API void setUVec3			(GLint location, glm::uvec3 const& list) const;
	FRAMEWORK_DLL_API void setMatrix		(GLint location, const glm::mat4x4& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setMatrix		(GLint location, const glm::mat3x3& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setImageUniform	(GLint location, int textureUnit) const;

public:
	FRAMEWORK_DLL_API void compile();

	// used by editor to re-read file paths
	FRAMEWORK_DLL_API void recompile();

private:
	GLuint m_id;
	std::string vShaderCode;
	std::string fShaderCode;

	std::string errorMessage;

	const char* vertexShaderPath = nullptr;
	const char* fragmentShaderPath = nullptr;
};
#pragma once

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <variant>
#include <unordered_map>
#include <glm/glm.hpp>

#include "type_alias.h"
#include "export.h"
#include "reflection.h"

// ================================================================================================================================
enum class Pipeline {
	PBR,			// uses everything.
	Color,			// only uses albedo.
};

class Texture;

#define AllUniformTypes \
	bool, int, unsigned int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, TypedResourceID<Texture>

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
	TypedResourceID<CustomShader> selectedShader;
	std::unordered_map<std::string, OverriddenUniformData> overridenUniforms;

	REFLECTABLE(
		selectedShader,
		overridenUniforms
	)
};
// ================================================================================================================================

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
	FRAMEWORK_DLL_API void setBool			(const std::string& name, bool value) const;
	FRAMEWORK_DLL_API void setInt			(const std::string& name, int value) const;
	FRAMEWORK_DLL_API void setUInt			(const std::string& name, unsigned int value) const;
	FRAMEWORK_DLL_API void setFloat			(const std::string& name, float value) const;
	FRAMEWORK_DLL_API void setVec2			(const std::string& name, float x, float y) const;
	FRAMEWORK_DLL_API void setVec2			(const std::string& name, glm::vec2 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(const std::string& name, glm::vec3 const& list) const;
	FRAMEWORK_DLL_API void setVec3			(const std::string& name, float x, float y, float z) const;
	FRAMEWORK_DLL_API void setVec4			(const std::string& name, glm::vec4 const& list) const;
	FRAMEWORK_DLL_API void setMatrix		(const std::string& name, const glm::mat4x4& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setMatrix		(const std::string& name, const glm::mat3x3& matrix, bool transpose = false) const;
	FRAMEWORK_DLL_API void setImageUniform	(const std::string& name, int uniform) const;

public:
	FRAMEWORK_DLL_API void compile();

private:
	GLuint m_id;
	std::string vShaderCode;
	std::string fShaderCode;

	std::string errorMessage;
};
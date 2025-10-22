#pragma once

#include "export.h"
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include "Resource/Material.h"
class Shader
{
public:
	enum class ShaderCompileStatus {
		Success,
		Failed
	}shaderCompileStatus;
public:
	FRAMEWORK_DLL_API Shader(const char* vertexPath, const char* fragmentPath);
	FRAMEWORK_DLL_API Shader(CustomShader customShader, Material::Pipeline const& selectedPipeline);
	FRAMEWORK_DLL_API ~Shader();

	Shader(Shader const& other) = delete;
	Shader(Shader&& other) = delete;
	Shader& operator=(Shader const& other) = delete;
	Shader& operator=(Shader&& other) = delete;

public:
	// accessor
	FRAMEWORK_DLL_API GLuint id() const;

	// use/activate the shader
	FRAMEWORK_DLL_API void use() const;
	FRAMEWORK_DLL_API void unuse() const;
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
private:
	void compile(const char* vShaderCode, const char* fShaderCode);
private:

	GLuint m_id;
};
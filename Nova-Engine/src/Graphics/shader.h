#pragma once

#include "export.h"
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	Shader(Shader const& other) = delete;
	Shader(Shader&& other) = delete;
	Shader& operator=(Shader const& other) = delete;
	Shader& operator=(Shader&& other) = delete;

public:
	// accessor
	GLuint id() const;

	// use/activate the shader
	void use() const;
	void unuse() const;

	// recompile shader based on vertexPath and fragmentPath
	ENGINE_DLL_API void compile();

	// retrieve filepaths
	const char* getVertexPath() const;
	const char* getFragmentPath() const;

public:
	// setting uniform functions
	void setBool			(const std::string& name, bool value) const;
	void setInt				(const std::string& name, int value) const;
	void setUInt			(const std::string& name, unsigned int value) const;
	void setFloat			(const std::string& name, float value) const;
	void setVec2			(const std::string& name, float x, float y) const;
	void setVec2			(const std::string& name, glm::vec2 const& list) const;
	void setVec3			(const std::string& name, glm::vec3 const& list) const;
	void setVec3			(const std::string& name, float x, float y, float z) const;
	void setVec4			(const std::string& name, glm::vec4 const& list) const;
	void setMatrix			(const std::string& name, const glm::mat4x4& matrix, bool transpose = false) const;
	void setMatrix			(const std::string& name, const glm::mat3x3& matrix, bool transpose = false) const;
	void setImageUniform	(const std::string& name, int uniform) const;

private:
	const char* vertexPath;
	const char* fragmentPath;

	GLuint m_id;
	bool initialised;
};
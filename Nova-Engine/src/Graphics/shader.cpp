#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) :
	initialised		{ },
	vertexPath		{ vertexPath },
	fragmentPath	{ fragmentPath }
{
	compile();
	initialised = true;
}

Shader::~Shader() {
	glDeleteProgram(m_id);
}

void Shader::use() const {
	glUseProgram(m_id);
}

void Shader::unuse() const {
	glUseProgram(0);
}

void Shader::compile() {
	if (initialised) {
		glDeleteProgram(m_id);
	}

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	// open files
	vShaderFile.open(vertexPath);
	fShaderFile.open(fragmentPath);

	std::stringstream vShaderStream, fShaderStream;

	// read file's buffer contents into streams
	vShaderStream << vShaderFile.rdbuf();
	fShaderStream << fShaderFile.rdbuf();

	// convert stream into string
	vertexCode = vShaderStream.str();
	fragmentCode = fShaderStream.str();

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2. compile shaders
	int success;

	// vertex Shader
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, nullptr);
	glCompileShader(vertex);

	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		//logger.error(std::string{ "Error! Compilation of vertex shader failed!" } + infoLog + '\n');
	}

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		//logger.error(std::string{ "Error! Compilation of fragment shader failed!" } + infoLog + '\n');

	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vertex);
	glAttachShader(m_id, fragment);
	glLinkProgram(m_id);

	// print linking errors if any
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		//logger.error(std::string{ "Error! Linking of shaders failed!" } + infoLog + '\n');
	}

	// delete shaders; they're linked into our program and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::setBool(const std::string& name, bool const value) const {
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int const value) const {
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float const value) const {
	glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, glm::vec2 const& list) const {
	glUniform2f(glGetUniformLocation(m_id, name.c_str()), list[0], list[1]);
}

void Shader::setMatrix(const std::string& name, const glm::mat4x4& matrix, bool transpose) const {
	glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void Shader::setImageUniform(const std::string& name, int uniform) const {
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), uniform);
}

const char* Shader::getVertexPath() const {
	return vertexPath;
}

const char* Shader::getFragmentPath() const {
	return fragmentPath;
}

void Shader::setVec3(const std::string& name, glm::vec3 const& list) const {
	glUniform3f(glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2]);
}

void Shader::setVec2(const std::string& name, float x, float y) const {
	glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, glm::vec4 const& list) const {
	glUniform4f(glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2], list[3]);
}

GLuint Shader::id() const {
	return m_id;
}
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "Logger.h"

#include "customShader.h"

#include <glad/glad.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath) 
	:	shaderCompileStatus	{ ShaderCompileStatus::Failed }	
	,	vShaderCode			{}
	,	fShaderCode			{}
	,	m_id				{}
{
	// retrieve the vertex/fragment source code from filePath
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
	vShaderCode = vShaderStream.str();
	fShaderCode = fShaderStream.str();

	compile();
}

Shader::Shader(std::string vertexCode, std::string fragmentCode)
	:	shaderCompileStatus	{ ShaderCompileStatus::Failed }
	,	vShaderCode			{ std::move(vertexCode) }
	,	fShaderCode			{ std::move(fragmentCode) }
	,	m_id				{}
{
	// Compile
	compile();
}

Shader::~Shader() {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}
}

Shader::Shader(Shader&& other) noexcept
	:	shaderCompileStatus	{ other.shaderCompileStatus }
	,	vShaderCode			{ std::move(other.vShaderCode) }
	,	fShaderCode			{ std::move(other.fShaderCode) }
	,	errorMessage		{ std::move(other.errorMessage) }
	,	m_id				{ other.m_id }
{
	other.shaderCompileStatus = ShaderCompileStatus::Failed;
}

Shader& Shader::operator=(Shader&& other) noexcept {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}

	shaderCompileStatus = other.shaderCompileStatus;
	vShaderCode			= std::move(other.vShaderCode);
	fShaderCode			= std::move(other.fShaderCode);
	errorMessage		= std::move(other.errorMessage);
	m_id				= other.m_id;

	other.shaderCompileStatus = ShaderCompileStatus::Failed;

	return *this;
}

void Shader::use() const {
	glUseProgram(m_id);
}

void Shader::unuse() const {
	glUseProgram(0);
}

std::string const& Shader::getVertexShader() const {
	return vShaderCode;
}

std::string const& Shader::getFragmentShader() const {
	return fShaderCode;
}

std::string const& Shader::getErrorMessage() const {
	return errorMessage;
}

bool Shader::hasCompiled() const {
	return shaderCompileStatus == Shader::ShaderCompileStatus::Success;
}

void Shader::compile() {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}

	int success;

	// vertex Shader
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexCStr = vShaderCode.c_str();
	glShaderSource(vertex, 1, &vertexCStr, nullptr);
	glCompileShader(vertex);

	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		Logger::error("Error! Compilation of vertex shader failed! {}", infoLog);
		errorMessage = std::string{ "ERROR!! Compilation of vertex shader failed! " } + infoLog;
		shaderCompileStatus = ShaderCompileStatus::Failed;
		return;
	}

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	const char * fragmentCStr = fShaderCode.c_str();
	glShaderSource(fragment, 1, &fragmentCStr, nullptr);
	glCompileShader(fragment);

	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		Logger::error("Error! Compilation of fragment shader failed! {}", infoLog);
		errorMessage = std::string{ "ERROR!! Compilation of fragment shader failed! " } + infoLog;
		shaderCompileStatus = ShaderCompileStatus::Failed;
		return;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vertex);
	glAttachShader(m_id, fragment);
	glLinkProgram(m_id);

	// print linking errors if any
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
		Logger::error("Error! Linking of shaders failed! {}", infoLog);
		errorMessage = std::string{ "ERROR!! Linking of shaders failed! " } + infoLog;
		shaderCompileStatus = ShaderCompileStatus::Failed;
		errorMessage = infoLog;
		return;
	}

	// delete shaders; they're linked into our program and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	shaderCompileStatus = ShaderCompileStatus::Success;
	errorMessage.clear();
}

void Shader::setBool(const std::string& name, bool const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setUInt(const std::string& name, unsigned int value) const {
	glProgramUniform1ui(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float const value) const {
	glProgramUniform1f(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, glm::vec2 const& list) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1]);
}

void Shader::setMatrix(const std::string& name, const glm::mat4x4& matrix, bool transpose) const {
	glProgramUniformMatrix4fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void Shader::setMatrix(const std::string& name, const glm::mat3x3& matrix, bool transpose) const {
	glProgramUniformMatrix3fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void Shader::setImageUniform(const std::string& name, int uniform) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), uniform);
}


void Shader::setVec3(const std::string& name, glm::vec3 const& list) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2]);
}

void Shader::setVec2(const std::string& name, float x, float y) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, glm::vec4 const& list) const {
	glProgramUniform4f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2], list[3]);
}

GLuint Shader::id() const {
	return m_id;
}
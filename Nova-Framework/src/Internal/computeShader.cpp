#include "computeShader.h"
#include "Logger.h"

#include <sstream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

ComputeShader::ComputeShader(const char* computeShaderPath)
	: shaderCompileStatus{ ShaderCompileStatus::Failed }
	, computeShaderCode{}
	, m_id{}
	, computeShaderPath{ computeShaderPath }
{
	recompile();
}

ComputeShader::~ComputeShader() {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}
}

ComputeShader::ComputeShader(ComputeShader&& other) noexcept
	: shaderCompileStatus{ other.shaderCompileStatus }
	, computeShaderCode{ std::move(other.computeShaderCode) }
	, errorMessage{ std::move(other.errorMessage) }
	, m_id{ other.m_id }
{
	other.shaderCompileStatus = ShaderCompileStatus::Failed;
}

ComputeShader& ComputeShader::operator=(ComputeShader&& other) noexcept {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}

	shaderCompileStatus = other.shaderCompileStatus;
	computeShaderCode = std::move(other.computeShaderCode);
	errorMessage = std::move(other.errorMessage);
	m_id = other.m_id;

	other.shaderCompileStatus = ShaderCompileStatus::Failed;

	return *this;
}

void ComputeShader::recompile() {
	if (computeShaderPath) {
		// retrieve the compute source code from filePath
		std::ifstream vComputeShaderFile;

		// ensure ifstream objects can throw exceptions
		vComputeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// open file
		vComputeShaderFile.open(computeShaderPath);

		std::stringstream vComputeShaderStream;

		// read file's buffer contents into streams
		vComputeShaderStream << vComputeShaderFile.rdbuf();

		// convert stream into string
		computeShaderCode = vComputeShaderStream.str();

		compile();
	}
}

void ComputeShader::compile() {
	if (shaderCompileStatus == ShaderCompileStatus::Success) {
		glDeleteProgram(m_id);
	}

	int success;

	// vertex Shader
	GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
	const char* computeCStr = computeShaderCode.c_str();
	glShaderSource(compute, 1, &computeCStr, nullptr);
	glCompileShader(compute);

	// print compile errors if any
	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(compute, 512, nullptr, infoLog);
		Logger::error("Error! Compilation of compute shader failed! {}", infoLog);
		errorMessage = std::string{ "ERROR!! Compilation of compute shader failed! " } + infoLog;
		shaderCompileStatus = ShaderCompileStatus::Failed;
		return;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, compute);
	glLinkProgram(m_id);

	// print linking errors if any
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
		Logger::error("Error! Linking of shader failed! {}", infoLog);
		errorMessage = std::string{ "ERROR!! Linking of shader failed! " } + infoLog;
		shaderCompileStatus = ShaderCompileStatus::Failed;
		errorMessage = infoLog;
		return;
	}

	// delete shaders; they're linked into our program and no longer necessary
	glDeleteShader(compute);
	shaderCompileStatus = ShaderCompileStatus::Success;
	errorMessage.clear();
}

void ComputeShader::use() const {
	glUseProgram(m_id);
}

void ComputeShader::unuse() const {
	glUseProgram(0);
}

void ComputeShader::setBool(const std::string& name, bool const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void ComputeShader::setInt(const std::string& name, int const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void ComputeShader::setUInt(const std::string& name, unsigned int value) const {
	glProgramUniform1ui(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void ComputeShader::setFloat(const std::string& name, float const value) const {
	glProgramUniform1f(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void ComputeShader::setVec2(const std::string& name, glm::vec2 const& list) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1]);
}

void ComputeShader::setMatrix(const std::string& name, const glm::mat4x4& matrix, bool transpose) const {
	glProgramUniformMatrix4fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void ComputeShader::setMatrix(const std::string& name, const glm::mat3x3& matrix, bool transpose) const {
	glProgramUniformMatrix3fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void ComputeShader::setImageUniform(const std::string& name, int uniform) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), uniform);
}

void ComputeShader::setVec3(const std::string& name, glm::vec3 const& list) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2]);
}

void ComputeShader::setVec2(const std::string& name, float x, float y) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y);
}

void ComputeShader::setVec3(const std::string& name, float x, float y, float z) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void ComputeShader::setVec4(const std::string& name, glm::vec4 const& list) const {
	glProgramUniform4f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2], list[3]);
}

void ComputeShader::setUVec2(const std::string& name, glm::uvec2 const& list) const {
	glProgramUniform2ui(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1]);
}

void ComputeShader::setUVec3(const std::string& name, glm::uvec3 const& list) const {
	glProgramUniform3ui(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2]);
}

GLuint ComputeShader::id() const {
	return m_id;
}
#include "customshader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Logger.h"


CustomShader::CustomShader(ResourceID id, ResourceFilePath resourceFilePath, BlendingConfig blendingConfig, DepthTestingMethod depthTestingMethod, const char* fShaderCode, std::unordered_map<ShaderVariableName, ShaderVariableType> const& uniforms)
	:Resource{id,resourceFilePath}
	,m_id{}
	,blendingConfig{blendingConfig}
	,depthTestingMethod{depthTestingMethod}
	,uniforms{uniforms}
{
	// To Do, a vector of ids to store compatible shaders
	//int success;

	//// vertex Shader
	//GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(vertex, 1, &vShaderCode, nullptr);
	//glCompileShader(vertex);

	//// print compile errors if any
	//glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	//if (!success) {
	//	char infoLog[512];
	//	glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
	//	Logger::error("Error! Compilation of vertex shader failed! {}", infoLog);
	//}

	//GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragment, 1, &fShaderCode, nullptr);
	//glCompileShader(fragment);

	//// print compile errors if any
	//glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	//if (!success) {
	//	char infoLog[512];
	//	glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
	//	Logger::error("Error! Compilation of fragment shader failed! {}", infoLog);
	//}

	//m_id = glCreateProgram();
	//glAttachShader(m_id, vertex);
	//glAttachShader(m_id, fragment);
	//glLinkProgram(m_id);

	//// print linking errors if any
	//glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	//if (!success) {
	//	char infoLog[512];
	//	glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
	//	Logger::error("Error! Linking of shaders failed! {}", infoLog);
	//}

	//// delete shaders; they're linked into our program and no longer necessary
	//glDeleteShader(vertex);
	//glDeleteShader(fragment);
}

CustomShader::~CustomShader(){
	/*glDeleteProgram(m_id);*/
}
void CustomShader::use(Pipeline pipeline) const {
	glUseProgram(m_id);
	switch (blendingConfig) {
	case BlendingConfig::AlphaBlending:
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
		break;
	case BlendingConfig::AdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BlendingConfig::PureAdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case BlendingConfig::PremultipliedAlpha:
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendingConfig::Disabled:
		glDisable(GL_BLEND);
		break;
	}
	switch (depthTestingMethod) {
	case DepthTestingMethod::DepthTest:
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		break;
	case DepthTestingMethod::NoDepthWrite:
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		break;
	case DepthTestingMethod::NoDepthWriteTest:
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		break;
	}
}

void CustomShader::unuse() const {
	glUseProgram(0);
}

void CustomShader::setBool(const std::string& name, bool const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void CustomShader::setInt(const std::string& name, int const value) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void CustomShader::setUInt(const std::string& name, unsigned int value) const {
	glProgramUniform1ui(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void CustomShader::setFloat(const std::string& name, float const value) const {
	glProgramUniform1f(m_id, glGetUniformLocation(m_id, name.c_str()), value);
}

void CustomShader::setVec2(const std::string& name, glm::vec2 const& list) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1]);
}

void CustomShader::setMatrix(const std::string& name, const glm::mat4x4& matrix, bool transpose) const {
	glProgramUniformMatrix4fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void CustomShader::setMatrix(const std::string& name, const glm::mat3x3& matrix, bool transpose) const {
	glProgramUniformMatrix3fv(m_id, glGetUniformLocation(m_id, name.c_str()), 1, transpose, glm::value_ptr(matrix));
}

void CustomShader::setImageUniform(const std::string& name, int uniform) const {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, name.c_str()), uniform);
}

void CustomShader::setVec3(const std::string& name, glm::vec3 const& list) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2]);
}

void CustomShader::setVec2(const std::string& name, float x, float y) const {
	glProgramUniform2f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y);
}

void CustomShader::setVec3(const std::string& name, float x, float y, float z) const {
	glProgramUniform3f(m_id, glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void CustomShader::setVec4(const std::string& name, glm::vec4 const& list) const {
	glProgramUniform4f(m_id, glGetUniformLocation(m_id, name.c_str()), list[0], list[1], list[2], list[3]);
}

GLuint CustomShader::id() const {
	return m_id;
}

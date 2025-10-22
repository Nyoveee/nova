#include "customshader.h"
#include "shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Logger.h"

#include "Material.h"

CustomShader::CustomShader(ResourceID id, ResourceFilePath resourceFilePath, ShaderParserData shaderData)
	: Resource{ id, resourceFilePath }
	, customShaderData{ shaderData } {}

CustomShader::~CustomShader(){}

void CustomShader::use(MaterialData const& data)
{
	if (!shaders[data.selectedPipeline]) {
		Logger::error("Unable to use custom shader, shader associated with the pipeline does not exist");
		return;
	}

	Shader& shader{ shaders[data.selectedPipeline].value() };
	if (shader.shaderCompileStatus == Shader::ShaderCompileStatus::Failed) {
		Logger::error("Unable to use custom shader, the shader associated with the pipeline failed to compile");
		return;
	}

	switch (customShaderData.blendingConfig) {
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

	switch (customShaderData.depthTestingMethod) {
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

	// Set up Uniforms
	for (auto const& [name, overriddenUniformData] : data.overridenUniforms) {
		if (overriddenUniformData.type == "bool")
			shader.setBool(name, std::get<bool>(overriddenUniformData.value));
		if (overriddenUniformData.type == "int")
			shader.setInt(name, std::get<int>(overriddenUniformData.value));
		if (overriddenUniformData.type == "uint")
			shader.setUInt(name, std::get<unsigned int>(overriddenUniformData.value));
		if (overriddenUniformData.type == "float")
			shader.setFloat(name, std::get<float>(overriddenUniformData.value));
		if (overriddenUniformData.type == "vec2")
			shader.setVec2(name, std::get<glm::vec2>(overriddenUniformData.value));
		if (overriddenUniformData.type == "vec3")
			shader.setVec3(name, std::get<glm::vec3>(overriddenUniformData.value));
		if (overriddenUniformData.type == "vec4")
			shader.setVec4(name, std::get<glm::vec4>(overriddenUniformData.value));
		if (overriddenUniformData.type == "mat3")
			shader.setMatrix(name, std::get<glm::mat3>(overriddenUniformData.value));
		if (overriddenUniformData.type == "mat4")
			shader.setMatrix(name, std::get<glm::mat4>(overriddenUniformData.value));
		if (overriddenUniformData.type == "sampler2D")
			shader.setImageUniform(name, std::get<int>(overriddenUniformData.value));
	}
	// Use the shader
	shader.use();
}

void CustomShader::compileWithPipeline(Material const& material)
{
	// Setup Shader Paths based on material
	std::string fShaderLibraryPath{};
	std::string vShaderPath{};

	switch (material.materialData.selectedPipeline) {
	case Pipeline::PBR:
		vShaderPath = "System/Shader/PBR.vert";
		fShaderLibraryPath = "System/Shader/Library/pbrlibrary.frag";
		break;
	}

	if (fShaderLibraryPath.empty()) {
		Logger::error("Unable to compile Pipeline does not have a library");
		return;
	}

	// Vertex Shader
	std::ifstream vShaderFile{};
	std::stringstream vShaderStream;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	vShaderFile.open(vShaderPath);

	vShaderStream << vShaderFile.rdbuf();
	std::string vertexCode = vShaderStream.str();

	// Fragment Shader
	std::ifstream fShaderLibraryFile{};
	std::stringstream fShaderLibraryStream;
	fShaderLibraryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderLibraryFile.open(fShaderLibraryPath);

	// Attach Custom Shader Code with library
	fShaderLibraryStream << fShaderLibraryFile.rdbuf() << "void main(){" << customShaderData.fShaderCode << '}';
	std::string fragmentCode = fShaderLibraryStream.str();

	std::optional<Shader> shader{ Shader{ std::move(vertexCode), std::move(fragmentCode) } };
	shaders.insert({ material.materialData.selectedPipeline, std::move(shader) });
}

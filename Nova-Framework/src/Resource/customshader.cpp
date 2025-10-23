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
	, customShaderData{ shaderData } 
{
	compile();
}

CustomShader::~CustomShader(){}

void CustomShader::compile()
{
	// free the current shader if available..
	if (shader) {
		shader.reset();
	}

	// ========================================================
	// We set up the appropriate vertex and fragment shader library files based on the current pipeline..
	// ========================================================
	std::string fShaderLibraryPath{};
	std::string vShaderPath{};

	switch (customShaderData.pipeline) {
	case Pipeline::PBR:
		vShaderPath = "System/Shader/PBR.vert";
		fShaderLibraryPath = "System/Shader/Library/pbrlibrary.frag";
		break;
	}

	if (fShaderLibraryPath.empty()) {
		Logger::error("Unable to compile Pipeline does not have a library");
		return;
	}

	// ========================================================
	// We retrieve the string contents from our vertex shader file..
	// ========================================================
	std::ifstream vShaderFile{};
	std::stringstream vShaderStream;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	vShaderFile.open(vShaderPath);

	vShaderStream << vShaderFile.rdbuf();
	std::string vertexCode = vShaderStream.str();

	// ========================================================
	// We retrieve the string contents from our fragment shader file..
	// ========================================================
	std::ifstream fShaderLibraryFile{};
	std::stringstream fShaderLibraryStream;
	fShaderLibraryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderLibraryFile.open(fShaderLibraryPath);
	fShaderLibraryStream << fShaderLibraryFile.rdbuf();
	
	// ========================================================
	// We attach the custom shader's uniforms as input..
	// ========================================================
	fShaderLibraryStream << "\n\n// !! ==========================================";
	fShaderLibraryStream << "\n// Custom Shader Properties";
	fShaderLibraryStream << "\n// !! ==========================================\n\n";

	for (auto&& [identifier, type] : customShaderData.uniforms) {
		fShaderLibraryStream << "uniform " << type << " " << identifier << ";\n";
	}

	// ========================================================
	// We attach the custom shader's fragment code and wrap it in main..
	// ========================================================
	fShaderLibraryStream << "\n\n// !! ==========================================";
	fShaderLibraryStream << "\n// Custom Shader Fragment Code";
	fShaderLibraryStream << "\n// !! ==========================================\n\n";
	fShaderLibraryStream << "void main(){\n" << customShaderData.fShaderCode << "\n}";
	std::string fragmentCode = fShaderLibraryStream.str();

	shader = Shader{ std::move(vertexCode), std::move(fragmentCode) };
}

std::optional<Shader> const& CustomShader::getShader() const {
	return shader;
}

Pipeline CustomShader::getPipeline() const {
	return customShaderData.pipeline;
}

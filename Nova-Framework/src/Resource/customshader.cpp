#include "customshader.h"
#include "shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Logger.h"

#include "Material.h"

CustomShader::CustomShader(ResourceID id, ResourceFilePath resourceFilePath, ShaderParserData shaderData)
	: Resource			{ id, resourceFilePath }
	, customShaderData	{ shaderData } 
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

	if (customShaderData.fShaderCode.empty()) {
		return;
	}

	// ========================================================
	// We set up the appropriate vertex and fragment shader library files based on the current pipeline..
	// ========================================================
	std::string fShaderLibraryPath{};
	std::string vShaderLibraryPath{};

	switch (customShaderData.pipeline) {
	case Pipeline::PBR:
		vShaderLibraryPath = "System/Shader/Library/pbrlibrary.vert";
		fShaderLibraryPath = "System/Shader/Library/pbrlibrary.frag";
		break;
	case Pipeline::Color:
		vShaderLibraryPath = "System/Shader/Library/colorlibrary.vert";
		fShaderLibraryPath = "System/Shader/Library/colorlibrary.frag";
		break;
	default:
		assert(false && "Unhandled pipeline.");
		return;
	}

	// ========================================================
	// We retrieve the string contents from our vertex shader library file..
	// ========================================================
	std::ifstream vShaderLibraryFile{};
	std::stringstream vShaderLibraryStream;
	vShaderLibraryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	vShaderLibraryFile.open(vShaderLibraryPath);

	vShaderLibraryStream << vShaderLibraryFile.rdbuf();

	// ========================================================
	// We attach the vertex shader's fragment code and wrap it in main..
	// ========================================================
	vShaderLibraryStream << "\n// !! ==========================================";
	vShaderLibraryStream << "\n// Custom Shader Vertex Code";
	vShaderLibraryStream << "\n// !! ==========================================\n\n";
	vShaderLibraryStream << "void main(){" << customShaderData.vShaderCode << "}";
	
	std::string vertexCode = vShaderLibraryStream.str();

	// ========================================================
	// We retrieve the string contents from our fragment shader library file..
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

	for (auto&& [type, identifier, value] : customShaderData.uniformDatas) {
		std::string comment;
		std::string glslType = type;

		// map our custom data types back to glsl primitive..
		auto iterator = customTypeToGlslPrimitive.find(type);

		if (iterator != customTypeToGlslPrimitive.end()) {
			glslType = iterator->second;
			comment = "// was " + type;
		}

		fShaderLibraryStream << "uniform " << glslType << " " << identifier << "; " << comment << '\n';
	}

	// ========================================================
	// We attach the custom shader's fragment code and wrap it in main..
	// ========================================================
	fShaderLibraryStream << "\n// !! ==========================================";
	fShaderLibraryStream << "\n// Custom Shader Fragment Code";
	fShaderLibraryStream << "\n// !! ==========================================\n\n";
	fShaderLibraryStream << "vec4 __internal__main__(){" << customShaderData.fShaderCode << "}\n\n";

	std::string fragmentCode = fShaderLibraryStream.str();

	shader = Shader{ std::move(vertexCode), std::move(fragmentCode) };

	// ========================================================
	// We cache all uniform location data..
	// ========================================================
	uniformLocations.clear();

	for (auto const& uniformData : customShaderData.uniformDatas) {
		uniformLocations.push_back(shader->getUniformLocation(uniformData.identifier.c_str()));
	}
}

std::optional<Shader> const& CustomShader::getShader() const {
	return shader;
}
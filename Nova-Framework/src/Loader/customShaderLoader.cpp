#include "loader.h"
#include "Logger.h"
std::optional<ResourceConstructor> ResourceLoader<CustomShader>::load(ResourceID id, ResourceFilePath const& resourceFilePath){
	Logger::info("Loading shader resource file {}", resourceFilePath.string);

	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse shader.");
		return std::nullopt;
	}
	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// Blending Config
		CustomShader::BlendingConfig blendingConfig;
		readFromFile(resourceFile, blendingConfig);
		if (!readNextByteIfNull(resourceFile))
			return std::nullopt;
		// Depth Testing Method
		CustomShader::DepthTestingMethod depthTestingMethod;
		readFromFile(resourceFile, depthTestingMethod);
		if (!readNextByteIfNull(resourceFile))
			return std::nullopt;
		// Uniforms
		size_t uniformSize{};
		readFromFile(resourceFile, uniformSize);
		if (!readNextByteIfNull(resourceFile))
			return std::nullopt;
		std::unordered_map<ShaderVariableName, ShaderVariableType> uniforms;
		for (size_t i{}; i < uniformSize;++i) {
			size_t variableNameSize{}, variableTypeSize{};
			std::string variableName{}, variableType{};
			readFromFile(resourceFile, variableNameSize);
			readFromFile(resourceFile, variableTypeSize);
			resourceFile.read(variableName.data(), variableNameSize);
			resourceFile.read(variableType.data(), variableTypeSize);
			if (!readNextByteIfNull(resourceFile))
				return std::nullopt;
			uniforms[variableName] = variableType;
		}
		// Fragment Shader Code
		size_t fShaderCodeSize;
		readFromFile(resourceFile, fShaderCodeSize);
		std::string fShaderCode;
		resourceFile.read(fShaderCode.data(), fShaderCodeSize);
		if (!readNextByteIfNull(resourceFile))
			return std::nullopt;
		// Returns a ResourceConstructor
		return { ResourceConstructor{[id, resourceFilePath, blendingConfig, depthTestingMethod,fShaderCode = std::move(fShaderCode), uniforms = std::move(uniforms)]() {
			return std::make_unique<CustomShader>(id, std::move(resourceFilePath),blendingConfig,depthTestingMethod,std::move(fShaderCode.c_str()),std::move(uniforms));
		}} };
	}
	catch (std::exception const& ex) {
		Logger::error("Failed to load resource, {}", ex.what());
		return std::nullopt;
	}
}
#include "loader.h"
#include "Logger.h"

#include "customShader.h"

#include "Serialisation/serialisation.h"

std::optional<ResourceConstructor> ResourceLoader<CustomShader>::load(ResourceID id, ResourceFilePath const& resourceFilePath){
	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse shader.");
		return std::nullopt;
	}
	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	CustomShader::ShaderParserData shaderData;
	Serialiser::deserializeFromJsonFile(shaderData, resourceFile);

	// Returns a ResourceConstructor
	return { ResourceConstructor{[id, resourceFilePath, shaderData = std::move(shaderData)]() {
		return std::make_unique<CustomShader>(id, std::move(resourceFilePath), std::move(shaderData));
	}} };
}
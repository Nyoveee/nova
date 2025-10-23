#include "loader.h"
#include "Logger.h"
#include "Serialisation/serialisation.h"

#include "Material.h"

std::optional<ResourceConstructor> ResourceLoader<Material>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	Logger::info("Loading material resource file {}", resourceFilePath.string);

	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse material.");
		return std::nullopt;
	}
	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	MaterialData materialData;
	Serialiser::deserializeFromJsonFile(materialData, resourceFile);

	// Returns a ResourceConstructor
	return { ResourceConstructor{[id, resourceFilePath, materialData = std::move(materialData)]() {
		return std::make_unique<Material>(id, std::move(resourceFilePath), std::move(materialData));
	}} };
}
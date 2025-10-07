#include "loader.h"
#include "logger.h"

std::optional<ResourceConstructor> ResourceLoader<ScriptAsset>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse model.");
		return std::nullopt;
	}

	std::string className;
	resourceFile >> className;

	return { {[id, resourceFilePath, className = std::move(className)] {
			return std::make_unique<ScriptAsset>(id, resourceFilePath, std::move(className));
	}} };

}
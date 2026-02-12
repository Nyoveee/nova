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
	bool AdminScript{};
	resourceFile >> AdminScript;

	return { {[id, resourceFilePath, className = std::move(className), AdminScript = std::move(AdminScript)] {
			return std::make_unique<ScriptAsset>(id, resourceFilePath, std::move(className), std::move(AdminScript));
	}} };

}
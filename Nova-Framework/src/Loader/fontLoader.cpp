#include "loader.h"
#include "logger.h"

std::optional<ResourceConstructor> ResourceLoader<Font>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse font.");
		return std::nullopt;
	}

	/*std::string className;
	resourceFile >> className;*/

	return { {[id, resourceFilePath] {
			return std::make_unique<Font>(id, resourceFilePath);
	}} };

}
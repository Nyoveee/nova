#include "loader.h"
#include "logger.h"
#include "video.h"

std::optional<ResourceConstructor> ResourceLoader<Video>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to load video resource file.");
		return std::nullopt;
	}

	// For now, the video loader simply creates a Video resource
	// You can extend this to read additional metadata from the resource file if needed
	// (e.g., video dimensions, duration, codec info, etc.)

	return { {[id, resourceFilePath] {
		return std::make_unique<Video>(id, resourceFilePath);
	}} };
}

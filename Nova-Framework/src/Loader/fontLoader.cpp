#include "loader.h"
#include "logger.h"

#include "Serialisation/deserializeFromBinary.h"

std::optional<ResourceConstructor> ResourceLoader<Font>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse font.");
		return std::nullopt;
	}

	Font::Data data;
	deserializeFromBinary(resourceFile, data);

	return { {[id, resourceFilePath, data = std::move(data)] {
		return std::make_unique<Font>(id, resourceFilePath, std::move(data));
	}} };

}
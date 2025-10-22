#include "loader.h"
#include "Logger.h"

#include "Serialisation/serialisation.h"

std::optional<ResourceConstructor> ResourceLoader<Controller>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	Logger::info("Loading controller resource file {}", resourceFilePath.string);

	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse controller.");
		return std::nullopt;
	}

	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	Controller::Data data;
	Serialiser::deserializeFromJsonFile(data, resourceFile);
	
	// returns a resource constructor
	return { ResourceConstructor{[id, resourceFilePath, data = std::move(data)]() {
		return std::make_unique<Controller>(id, std::move(resourceFilePath), std::move(data));
	}} };
}
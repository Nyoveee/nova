#include "loader.h"
#include "Logger.h"
#include "Serialisation/serialisation.h"

std::optional<ResourceConstructor> ResourceLoader<Sequencer>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string };

	if (!resourceFile) {
		Logger::error("Failed to parse material.");
		return std::nullopt;
	}
	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	Sequencer::Data data;
	Serialiser::deserializeFromJsonFile(data, resourceFile);

	// Returns a ResourceConstructor
	return { ResourceConstructor{[id, resourceFilePath, data = std::move(data)]() {
		return std::make_unique<Sequencer>(id, std::move(resourceFilePath), std::move(data));
	}} };
}
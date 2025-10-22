#include "loader.h"
#include "Logger.h"

#include "Serialisation/serialisation.h"

std::optional<ResourceConstructor> ResourceLoader<Controller>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	Logger::info("Loading controller resource file {}", resourceFilePath.string);

	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse controller.");
		return std::nullopt;
	}

	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	Controller::Data data;

	reflection::visit(
		[&](auto&& fieldData) {
			auto&& dataMember = fieldData.get();
			using DataMemberType = std::decay_t<decltype(dataMember)>;

			Serialiser::deserializeFromJsonFile<DataMemberType>(dataMember, resourceFile);
		},
	data);
	
	// returns a resource constructor
	return { ResourceConstructor{[id, resourceFilePath, data = std::move(data)]() {
		return std::make_unique<Controller>(id, std::move(resourceFilePath), std::move(data));
	}} };
}
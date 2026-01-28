#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include "loader.h"
#include "Logger.h"

#include "model.h"

#include "Serialisation/deserializeFromBinary.h"

std::optional<ResourceConstructor> ResourceLoader<Model>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	std::ifstream resourceFile{ resourceFilePath.string, std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to parse model.");
		return std::nullopt;
	}

	resourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	ModelData modelData;
		
	reflection::visit(
		[&](auto&& fieldData) {
			auto&& dataMember = fieldData.get();
			using DataMemberType = std::decay_t<decltype(dataMember)>;

			deserializeFromBinary<DataMemberType>(resourceFile, dataMember);
		},
	modelData);

	// returns a resource constructor
	return { ResourceConstructor{[id, resourceFilePath, modelData = std::move(modelData)]() {
		return std::make_unique<Model>(id, std::move(resourceFilePath), std::move(modelData));
	}} };
}
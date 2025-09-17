#include "loader.h"

std::optional<ResourceConstructor> ResourceLoader<ScriptAsset>::load(ResourceID id, ResourceFilePath const& resourceFilePath) {
	return std::nullopt;
}
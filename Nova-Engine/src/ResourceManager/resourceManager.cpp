#include "resourceManager.h"
#include "Logger.h"
#include "descriptor.h"

ResourceManager::ResourceManager() {
	try {
		// ========================================
		// 1. Check if the resource directory exist, and the respective sub assets folder exist.
		// ========================================
		
		// Checking if the main resource directory exist.
		if (!std::filesystem::exists(Descriptor::resourceDirectory)) {
			std::filesystem::create_directory(Descriptor::resourceDirectory);
		}

		// Checking if the sub directories exist..
		for (auto&& [_, subResourceDirectory] : Descriptor::subResourceDirectories) {
			if (!std::filesystem::exists(subResourceDirectory)) {
				std::filesystem::create_directory(subResourceDirectory);
			}
		}

		// ========================================
		// 2. Load all the resources.
		// ========================================
		loadAllResources<Texture>();
		loadAllResources<Model>();
		loadAllResources<CubeMap>();
		loadAllResources<ScriptAsset>();
		loadAllResources<Model>();
}
	catch (const std::filesystem::filesystem_error& ex) {
		Logger::error("Filesystem error: {}", ex.what());
	}
}

Asset* ResourceManager::getResourceInfo(ResourceID id) {
	auto iterator = resources.find(id);
	
	if (iterator == std::end(resources)) {
		return nullptr;
	}
	
	auto&& [_, resource] = *iterator;
	return resource.get();
}

bool ResourceManager::doesResourceExist(ResourceID id) const {
	return resources.find(id) != resources.end();
}

ResourceID ResourceManager::getResourceID(std::filesystem::path const& path) const {
	auto iterator = filepathToResourceId.find(path.string());

	if (iterator == filepathToResourceId.end()) {
		return INVALID_ASSET_ID;
	}

	auto&& [_, resourceId] = *iterator;
	return resourceId;
}

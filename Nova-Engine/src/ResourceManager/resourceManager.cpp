#include "resourceManager.h"
#include "Logger.h"

ResourceManager::ResourceManager() :
	resourceDirectory	{ std::filesystem::current_path() / "Resources" },
	textureDirectory	{ resourceDirectory / "Texture" },
	modelDirectory		{ resourceDirectory / "Model" }
{
	try {
		// ========================================
		// 1. Check if the resource directory exist, and the respective sub assets folder exist.
		// ========================================
		
		// Checking if the main resource directory exist.
		if (!std::filesystem::exists(resourceDirectory)) {
			std::filesystem::create_directory(resourceDirectory);
		}

		// Checking if the sub folders Texture and Model exist.
		if (!std::filesystem::exists(textureDirectory)) {
			std::filesystem::create_directory(textureDirectory);
		}

		if (!std::filesystem::exists(modelDirectory)) {
			std::filesystem::create_directory(modelDirectory);
		}

		// ========================================
		// 2. Load all the resources.
		// ========================================
		loadAllResources<Texture>(textureDirectory);
		loadAllResources<Model>(modelDirectory);
	
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

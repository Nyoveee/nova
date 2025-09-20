#include "resourceManager.h"
#include "Logger.h"

#include "cubemap.h"
#include "scriptAsset.h"

#include "family.h"

ResourceManager::ResourceManager() {
	try {
		// ========================================
		// 1. Check if the resource directory exist, and the respective sub assets folder exist.
		// ========================================
		
		// Checking if the main resource directory exist.
		if (!std::filesystem::exists(AssetIO::resourceDirectory)) {
			std::filesystem::create_directory(AssetIO::resourceDirectory);
		}

		// Checking if the sub directories exist..
		for (auto&& [_, subResourceDirectory] : AssetIO::subResourceDirectories) {
			if (!std::filesystem::exists(subResourceDirectory)) {
				std::filesystem::create_directory(subResourceDirectory);
			}
		}

		// ========================================
		// 2. Record all resources..
		// ========================================
		recordAllResources<ALL_RESOURCES>();

}
	catch (const std::filesystem::filesystem_error& ex) {
		Logger::error("Filesystem error: {}", ex.what());
	}
}

void ResourceManager::update() {
	std::lock_guard lock{ initialisationQueueMutex };
	
	while (initialisationQueue.size()) {
		std::function<void()> callback = std::move(initialisationQueue.front());
		initialisationQueue.pop();
		callback();
	}
}

bool ResourceManager::doesResourceExist(ResourceID id) const {
	return resourceFilePaths.find(id) != resourceFilePaths.end();
}

void ResourceManager::submitInitialisationCallback(std::function<void()> callback) {
	std::lock_guard lock{ initialisationQueueMutex };
	initialisationQueue.push(std::move(callback));
}

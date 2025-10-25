#include "resourceManager.h"
#include "Logger.h"

#include "cubemap.h"
#include "scriptAsset.h"

#include "family.h"

#include "Profiling.h"

#include "Material.h"
#include "customShader.h"

ResourceManager::ResourceManager() {
	try {
		// ========================================
		// 1. Load all system resources..
		// ========================================
		loadAllSystemResources();

		// ========================================
		// 2. Check if the resource directory exist, and the respective sub assets folder exist.
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
		// 3. Load all resources..
		// ========================================
		loadAllResources<ALL_RESOURCES>();
	}
	catch (const std::filesystem::filesystem_error& ex) {
		Logger::error("Filesystem error: {}", ex.what());
	}
}

void ResourceManager::update() {
	ZoneScoped;

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

void ResourceManager::removeResource(ResourceID id) {
	resourceFilePaths.erase(id);
	loadedResources.erase(id);

	for (auto&& [resourceTypeId, resourceIds] : resourcesByType) {
		auto iterator = std::ranges::find(resourceIds, id);
		
		if (iterator != resourceIds.end()) {
			resourceIds.erase(iterator);
		}
	}
}

void ResourceManager::loadAllSystemResources() {
	for (auto&& [id, resourceFilePath] : AssetIO::systemModelResources) {
		addResourceFile<Model>(resourceFilePath, id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemMaterialResources) {
		addResourceFile<Material>(resourceFilePath, id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemShaderResources) {
		addResourceFile<CustomShader>(resourceFilePath, id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemTextureResources) {
		addResourceFile<Texture>(resourceFilePath, id);
	}
}
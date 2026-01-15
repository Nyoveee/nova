#include "resourceManager.h"
#include "Logger.h"

#include "equirectangularMap.h"
#include "scriptAsset.h"

#include "family.h"

#include "Profiling.h"

#include "Material.h"
#include "customShader.h"

ResourceManager::ResourceManager() {
	reload();
}

void ResourceManager::reload() {
	resourceFilePaths.clear();
	loadedResources.clear();
	createdResourceInstances.clear();
	resourcesByType.clear();

	try {
		// ========================================
		// 1. Load all system resources..
		// ========================================
		loadAllSystemResources();

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
#if defined(DEBUG)
	ZoneScoped;
#endif

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
		loadResource<Model>(id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemMaterialResources) {
		addResourceFile<Material>(resourceFilePath, id);
		loadResource<Material>(id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemShaderResources) {
		addResourceFile<CustomShader>(resourceFilePath, id);
		loadResource<CustomShader>(id);
	}

	for (auto&& [id, resourceFilePath] : AssetIO::systemTextureResources) {
		addResourceFile<Texture>(resourceFilePath, id);
		loadResource<Texture>(id);
	}
}

void ResourceManager::removeAllResourceInstance() {
	for (ResourceID id : createdResourceInstances) {
		removeResource(id);
	}

	createdResourceInstances.clear();
}
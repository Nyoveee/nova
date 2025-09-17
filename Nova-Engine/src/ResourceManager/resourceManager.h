	#pragma once

#include "export.h"
#include "asset.h"

#include "loader.h"

#include <concepts>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <queue>
#include <mutex>
#include <functional>

class AssetManager;

class ResourceManager {
public:
	enum class QueryResult {
		Success,		// if this enum is return, the pointer is valid

		Invalid,		// is never recorded in asset manager
		WrongType,		// asset exist and is loaded, but is not of type T. (most likely code error, this shouldn't happen)
		Loading,		// exist but is not loaded, asset manager is attempting to load it now.
		LoadingFailed	// there was an attempt to load the asset but it failed.
	};

	template <ValidAsset T>
	struct ResourceQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;
		QueryResult result;
	};

public:
	ENGINE_DLL_API ResourceManager();

	ENGINE_DLL_API ~ResourceManager()											= default;
	ENGINE_DLL_API ResourceManager(ResourceManager const& other)				= delete;
	ENGINE_DLL_API ResourceManager(ResourceManager&& other)						= delete;
	ENGINE_DLL_API ResourceManager& operator=(ResourceManager const& other)		= delete;
	ENGINE_DLL_API ResourceManager& operator=(ResourceManager&& other)			= delete;

	// main way all systems query for a specific resource.
	template <ValidAsset T>
	ResourceQuery<T> getResource(ResourceID id);

	// retrieve all resource ids of a given type.
	template <ValidAsset T>
	std::vector<ResourceID> const& getAllResources() const;

	// retrieve 1 resource id of a given type. (should never be invalid resource id, but still should handle the chance of it being invalid).
	template <ValidAsset T>
	ResourceID getSomeResourceID() const;

	// given an resource id, is the original resource type of T?
	template <ValidAsset T>
	bool isResource(ResourceID id) const;

public:
	ENGINE_DLL_API void update();
	ENGINE_DLL_API bool doesResourceExist(ResourceID id) const;

	ENGINE_DLL_API void submitInitialisationCallback(std::function<void()> callback);

private:
	friend AssetManager;

	// parses a given resource file. returns a valid resource id if its valid,
	// INVALID_ASSET_ID otherwise.
	template <ValidAsset T>
	ResourceID addResourceFile(ResourceFilePath const& filepath);

	// records all the given resources in a given directory, taking note of their filepaths.
	template <ValidAsset T>
	void recordAllResources();

private:
	// records all resource filepath and it's associated resource id.
	// we don't load assets at resource startup, only keeping track of resource filepath for us to load.
	std::unordered_map<ResourceID, ResourceFilePath> resourceFilePaths;

	// main container containing all LOADED resources. when an resource is loaded, it goes here.
	std::unordered_map<ResourceID, std::unique_ptr<Asset>> loadedResources;

	// groups all assets based on their type.
	std::unordered_map<ResourceTypeID, std::vector<ResourceID>> resourcesByType;

	// initialisationQueue represents the resources after the loading operation has been completed.
	// this function will construct the aset type, and store it in resources, the main container containing all LOADED resources.
	std::mutex initialisationQueueMutex;
	std::queue<std::function<void()>> initialisationQueue;
};

#include "resourceManager.ipp"
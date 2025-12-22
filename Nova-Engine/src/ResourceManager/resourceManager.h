#pragma once

#include "export.h"
#include "resource.h"

#include "loader.h"

#include <concepts>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <queue>
#include <mutex>
#include <functional>
#include <BS_thread_pool.hpp>

#include "Material.h"

class AssetManager;
class AssetViewerUI;

// some assets are lazily loaded, meaning the asset is loaded in a separate thread..
template<class T>
concept LazyLoadResource = std::same_as<T, Texture>;

class ResourceManager {
public:
	enum class QueryResult {
		Success,		// if this enum is return, the pointer is valid

		Invalid,		// is never recorded in asset manager
		WrongType,		// asset exist and is loaded, but is not of type T. (most likely code error, this shouldn't happen)
		Loading,		// exist but is not loaded, asset manager is attempting to load it now.
		LoadingFailed	// there was an attempt to load the asset but it failed.
	};

	template <ValidResource T>
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

public:
	// main way all systems query for a specific resource.
	// lazy loaded resources are loaded in a separate thread
	template <ValidResource T>
	ResourceQuery<T> getResource(ResourceID id);

	// loads the resource into memory. forces the resource to be loaded (no lazy loading) if not already loading
	// useful to force all system resources to load prior to launch.
	template <ValidResource T>
	void loadResource(ResourceID id);

	// only get resource if its already loaded. (mainly used for serialisation)
	template <ValidResource T>
	T* getResourceOnlyIfLoaded(ResourceID id);

	// retrieve all resource ids of a given type.
	template <ValidResource T>
	std::vector<ResourceID> const& getAllResources() const;

	// retrieve 1 resource id of a given type. (should never be invalid resource id, but still should handle the chance of it being invalid).
	template <ValidResource T>
	ResourceID getSomeResourceID() const;

	// given an resource id, is the original resource type of T?
	template <ValidResource T>
	bool isResource(ResourceID id) const;

	// Creates a copy of this resource instance in memory..
	template <ValidResource T>
	ResourceID createResourceInstance(ResourceID id);

	// Removes all resource instances..
	void removeAllResourceInstance();

public:
	// Reloads all resources..
	ENGINE_DLL_API void reload();

	ENGINE_DLL_API void update();
	ENGINE_DLL_API bool doesResourceExist(ResourceID id) const;

	ENGINE_DLL_API void submitInitialisationCallback(std::function<void()> callback);

private:
	friend AssetManager;
	friend AssetViewerUI;

	// parses a given resource file. returns a valid resource id if its valid,
	// you can provide an resource id for the resource manager to use. (mainly used for system resources)
	template <ValidResource T>
	ResourceID addResourceFile(ResourceFilePath const& filepath, ResourceID id = INVALID_RESOURCE_ID);

	// given a resource constructor, loads it into the resource manager
	// resource constructors are returned by resource loaders.
	template <ValidResource T>
	T* loadResourceConstructor(std::optional<ResourceConstructor> resourceConstructor);

	// records all the given resources in a given directory, taking note of their filepaths.
	template<ValidResource ...T>
	void loadAllResources();

	// records all the given system resources in a given directory, taking note of their filepaths.
	void loadAllSystemResources();

	// this is only called by the asset manager to remove outdated resources. (housekeeping)
	ENGINE_DLL_API void removeResource(ResourceID id);

private:
	// records all resource filepath and it's associated resource id.
	// we don't load assets at resource startup, only keeping track of resource filepath for us to load.
	std::unordered_map<ResourceID, ResourceFilePath> resourceFilePaths;

	// main container containing all LOADED resources. when an resource is loaded, it goes here.
	// this also include resource instances..
	std::unordered_map<ResourceID, std::unique_ptr<Resource>> loadedResources;

	// stores all copies of resources instances. this is useful so that we can clear them at the end of simulation.
	std::vector<ResourceID> createdResourceInstances;

	// keeps track of all resource that is loaded in a separate thread. let's not load it twice.
	std::unordered_set<ResourceID> resourceIsLoading;

	// groups all assets based on their type.
	std::unordered_map<ResourceTypeID, std::vector<ResourceID>> resourcesByType;

	// initializationQueue represents the resources after the loading operation has been completed.
	// this function will construct the asset type, and store it in resources, the main container containing all LOADED resources.
	std::mutex initialisationQueueMutex;
	std::queue<std::function<void()>> initialisationQueue;

	BS::thread_pool<BS::tp::none> threadPool;
};

#include "resourceManager.ipp"
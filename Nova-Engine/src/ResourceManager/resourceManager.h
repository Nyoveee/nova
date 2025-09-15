	#pragma once

#include "export.h"
#include "asset.h"

#include "texture.h"
#include "model.h"
#include "audio.h"
#include "cubemap.h"
#include "scriptAsset.h"

#include <concepts>
#include <unordered_map>
#include <memory>
#include <filesystem>

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

public:
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
	// this is only used to get metadata / info about the resources (like name, is asset loaded..)
	// this doesnt not load the resource!!
	// since there is no loading of resource, you retrieve the data instantly.
	ENGINE_DLL_API Asset* getResourceInfo(ResourceID id);

	// get the resource id for a given resource filepath. may be INVALID_ASSET_ID.
	ENGINE_DLL_API ResourceID getResourceID(ResourceFilePath const& path) const;

	ENGINE_DLL_API bool doesResourceExist(ResourceID id) const;

private:
	friend AssetManager;

	// parses a given resource file.
	template <ValidAsset T>
	T* addResourceFile(ResourceFilePath const& filepath);

	// template <ValidAsset T>
	// void addResourceFile(AssetInfo<T> assetInfo);

	// loads all the given resources in a given directory,
	template <ValidAsset T>
	void loadAllResources();

private:
	// main container containing all assets.
	std::unordered_map<ResourceID, std::unique_ptr<Asset>> resources;

	// groups all assets based on their type.
	std::unordered_map<ResourceTypeID, std::vector<ResourceID>> resourcesByType;

	// associates an resource id with the corresponding resource type.
	std::unordered_map<ResourceID, ResourceTypeID> resourceIdToType;
	
	// maps filepath to resource ID.
	std::unordered_map<ResourceFilePath, ResourceID> filepathToResourceId;
};

#include "resourceManager.ipp"
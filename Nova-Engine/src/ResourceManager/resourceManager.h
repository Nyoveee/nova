#pragma once

#include "export.h"
#include "AssetManager/Asset/asset.h"

#include <concepts>
#include <unordered_map>
#include <memory>
#include <filesystem>

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
	struct AssetQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;
		QueryResult result;
	};

public:
	DLL_API ResourceManager();

	DLL_API ~ResourceManager()											= default;
	DLL_API ResourceManager(ResourceManager const& other)				= delete;
	DLL_API ResourceManager(ResourceManager&& other)					= delete;
	DLL_API ResourceManager& operator=(ResourceManager const& other)	= delete;
	DLL_API ResourceManager& operator=(ResourceManager&& other)			= delete;

public:
	// the king.
	template <ValidAsset T>
	Asset& addAsset();

private:
	std::filesystem::path resourceDirectory;

	// main container containing all assets.
	std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;
};

#include "resourceManager.ipp"
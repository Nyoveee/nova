#pragma once

#include "export.h"

#include <memory>
#include <unordered_map>

#include "AssetManager/Asset/asset.h"
#include "AssetManager/Asset/texture.h"

// Assets stored in the asset manager merely point to these assets in file location.
// These assets could be loaded or not loaded.
// Asset Manager merely acts as a bookkeep to all possible assets in the engine.

class AssetManager {
public:
	template <typename T>
	struct AssetQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;

		enum Result {
			Success,	
			Invalid,	// is never recorded in asset manager
			WrongType,	// asset exist and is loaded, but is not of type T. (most likely code error, this shouldn't happen)
			Loading		// exist but is not loaded, asset manager is attempting to load it now.
		} result;
	};

public:
	DLL_API AssetManager();

public:
	template <typename T, typename ...Args> requires std::derived_from<T, Asset>
	void addAsset(std::string filepath, Args... args);

	template <typename T> requires std::derived_from<T, Asset>
	AssetQuery<T> getAsset(AssetID id);

private:
	std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;
};

#include "AssetManager/assetMananger.ipp"
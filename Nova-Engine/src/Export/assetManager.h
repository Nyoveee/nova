#pragma once

#include "export.h"
#include <memory>
#include <unordered_map>

#include "AssetManager/Asset/asset.h"
#include "AssetManager/Asset/texture.h"
#include "AssetManager/Asset/model.h"
#include "AssetManager/folder.h"

// Assets stored in the asset manager merely point to these assets in file location.
// These assets could be loaded or not loaded.
// Asset Manager merely acts as a bookkeep to all possible assets in the engine.

class AssetManager {
public:
	enum class QueryResult {
		Success,
		Invalid,	// is never recorded in asset manager
		WrongType,	// asset exist and is loaded, but is not of type T. (most likely code error, this shouldn't happen)
		Loading		// exist but is not loaded, asset manager is attempting to load it now.
	};

	template <typename T>
	struct AssetQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;
		QueryResult result;
	};

public:
	DLL_API AssetManager();

public:
	// =========================================================
	// Retrieving assets and info..
	// =========================================================
	template <typename T> requires std::derived_from<T, Asset>
	AssetQuery<T> getAsset(AssetID id);

	// this is only used to get metadata / info about the assets (like name, is asset loaded..)
	// this doesnt not load the asset!!
	// since there is no loading of asset, you retrieve the data instantly.
	DLL_API Asset* getAssetInfo(AssetID id);

public:
	// =========================================================
	// Getters (no setters!)
	// =========================================================
	DLL_API std::unordered_map<FolderID, Folder> const& getDirectories() const;
	DLL_API std::vector<FolderID> const& getRootDirectories() const;

private:
	// =========================================================
	// Parsing of the assets directory..
	// =========================================================
	template <typename T, typename ...Args> requires std::derived_from<T, Asset>
	void addAsset(AssetID id, std::string filepath, Args... args);

	void recordFolder(
		FolderID folderId, 
		std::filesystem::path const& path, 
		std::filesystem::path const& assetDirectory
	);

	void parseAssetFile(
		std::filesystem::path const& path
	);

	template <typename T> requires std::derived_from<T, Asset>
	void recordAssetFile(
		std::filesystem::path const& path
	);

	AssetID generateAssetID(std::filesystem::path const& path);

private:
	std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;

	// Folder metadata (for tree traversal)
	std::unordered_map<FolderID, Folder> directories;
	std::vector<FolderID> rootDirectories;
	std::unordered_map<std::string, FolderID> folderNameToId;
};

#include "AssetManager/assetMananger.ipp"
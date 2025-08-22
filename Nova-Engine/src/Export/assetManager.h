#pragma once

#include <spdlog/spdlog.h>

#include "export.h"

#include <memory>
#include <unordered_map>
#include <optional>
#include <functional>

#include "AssetManager/Asset/asset.h"
#include "AssetManager/Asset/texture.h"
#include "AssetManager/Asset/model.h"
#include "AssetManager/folder.h"

// Assets stored in the asset manager merely point to these assets in file location.
// These assets could be loaded or not loaded.
// Asset Manager merely acts as a bookkeep to all possible assets in the engine.

// A valid asset must
// 1. Inherit from base class asset
// 2. Corresponding asset info inherit from BasicAssetInfo
//
// dont remove this concept! it's meant to save you from shooting yourself in the foot.
template <typename T>
concept ValidAsset = std::derived_from<T, Asset> && std::derived_from<AssetInfo<T>, BasicAssetInfo>;

#include "AssetManager/serialiseAssetFunctor.h"

class AssetManager {
public:
	enum class QueryResult {
		Success,
		Invalid,	// is never recorded in asset manager
		WrongType,	// asset exist and is loaded, but is not of type T. (most likely code error, this shouldn't happen)
		Loading		// exist but is not loaded, asset manager is attempting to load it now.
	};

	template <ValidAsset T>
	struct AssetQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;
		QueryResult result;
	};

public:
	DLL_API AssetManager();

	DLL_API ~AssetManager();
	DLL_API AssetManager(AssetManager const& other)				= delete;
	DLL_API AssetManager(AssetManager&& other)					= delete;
	DLL_API AssetManager& operator=(AssetManager const& other)	= delete;
	DLL_API AssetManager& operator=(AssetManager&& other)		= delete;

public:
	// =========================================================
	// Retrieving assets and info..
	// =========================================================
	template <ValidAsset T>
	AssetQuery<T> getAsset(AssetID id);

	// this is only used to get metadata / info about the assets (like name, is asset loaded..)
	// this doesnt not load the asset!!
	// since there is no loading of asset, you retrieve the data instantly.
	DLL_API Asset* getAssetInfo(AssetID id);

	// retrieve all assets of a given type.
	template <ValidAsset T>
	std::vector<std::reference_wrapper<Asset>> const& getAllAssets() const;

	// retrieve 1 asset id of a given type. (should never be invalid asset id, but still should handle the chance of it being invalid).
	template <ValidAsset T>
	AssetID getSomeAssetID() const;
	
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

	// the king.
	template <ValidAsset T>
	Asset& addAsset(AssetInfo<T> const& assetInfo);

	void recordFolder(
		FolderID folderId, 
		std::filesystem::path const& path, 
		std::filesystem::path const& assetDirectory
	);

	void parseAssetFile(
		std::filesystem::path const& path
	);

	template <ValidAsset T>
	void recordAssetFile(
		std::filesystem::path const& path
	);

	AssetID generateAssetID(std::filesystem::path const& path);

	// =========================================================
	// Parsing meta data file associated with the asset.
	// Each asset will contain a generic metadata (id and name)
	// Each specific type asset has additional metadata as well.
	// =========================================================
	
	// ==== Parse specific a specific asset type. ====
	// These functions will invoke the general functions below first which parses generic metadata info first
	// before performing additional parsing based on asset type.
	template <ValidAsset T>
	AssetInfo<T> parseMetaDataFile(std::filesystem::path const& path);
	
	template <ValidAsset T>
	AssetInfo<T> createMetaDataFile(std::filesystem::path const& path);

	// === Parse generic metadata info. These functions are invoked by the functions above first. ====
	
	// optional because parsing may fail.
	std::optional<BasicAssetInfo> parseMetaDataFile(std::filesystem::path const& path, std::ifstream& metaDataFile);
	BasicAssetInfo createMetaDataFile(std::filesystem::path const& path, std::ofstream& metaDataFile);

	// =========================================================
	// Serialising asset meta data..
	template <ValidAsset T>
	friend struct SerialiseAssetFunctor;

	template <ValidAsset T>
	void serialiseAssetMetaData(T const& asset);
	void serialiseAssetMetaData(Asset const& asset, std::ofstream& metaDataFile);

private:
	std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;
	
	// groups all assets based on their type.
	// reference wrapper because i dont want a chance of null pointer.
	std::unordered_map<AssetTypeID, std::vector<std::reference_wrapper<Asset>>> assetsByType;
	
	// welcome to template metaprogramming black magic.
	std::unordered_map<AssetID, std::unique_ptr<SerialiseAsset>> serialiseAssetFunctors;

	// Folder metadata (for tree traversal)
	std::unordered_map<FolderID, Folder> directories;
	std::vector<FolderID> rootDirectories;
	std::unordered_map<std::string, FolderID> folderNameToId;
};

#include "AssetManager/assetMananger.ipp"
#include "AssetManager/serialiseAssetFunctor.ipp"
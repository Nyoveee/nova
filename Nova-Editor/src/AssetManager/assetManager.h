#pragma once
#include "BS_thread_pool.hpp"

#include <memory>
#include <unordered_map>
#include <optional>
#include <functional>
#include <queue>
#include <mutex>
#include <atomic>

#include "asset.h"
#include "texture.h"
#include "model.h"
#include "audio.h"

#include "scriptAsset.h"
#include "AssetManager/folder.h"
#include "AssetManager/AssetDirectoryWatcher.h"

#include "Logger.h"

#include "AssetManager/assetFunctor.h"

class ResourceManager;
class Engine;

class AssetManager {
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

	struct Filepaths {
		DescriptorFilePath descriptorFilepath;
		ResourceFilePath resourceFilepath;
	};

public:
	AssetManager(ResourceManager& resourceManager, Engine& engine);

	~AssetManager();
	AssetManager(AssetManager const& other)				= delete;
	AssetManager(AssetManager&& other)					= delete;
	AssetManager& operator=(AssetManager const& other)	= delete;
	AssetManager& operator=(AssetManager&& other)		= delete;

public:
#if 0
	// The asset manager needs to check if there are any completed queue request
	// that needs to be done on the main thread.
	void update();

	// =========================================================
	// Retrieving assets and info..
	// =========================================================
	template <ValidAsset T>
	AssetQuery<T> getAsset(AssetID id);

	// this is only used to get metadata / info about the assets (like name, is asset loaded..)
	// this doesnt not load the asset!!
	// since there is no loading of asset, you retrieve the data instantly.
	DLL_API Asset* getAssetInfo(AssetID id);

	// retrieve all asset ids of a given type.
	template <ValidAsset T>
	std::vector<AssetID> const& getAllAssets() const;

	// retrieve 1 asset id of a given type. (should never be invalid asset id, but still should handle the chance of it being invalid).
	template <ValidAsset T>
	AssetID getSomeAssetID() const;
	
	// given an asset id, is the original asset type of T?
	template <ValidAsset T>
	bool isAsset(AssetID id) const;

	// attempts to retrieve asset id given full file path
	std::optional<AssetID> getAssetId(std::string const& filepath) const;

public:
	// =========================================================
	// Getters (no setters!)
	// =========================================================

	DLL_API void getAssetLoadingStatus() const;

private:
	// =========================================================
	// Parsing of the assets directory..
	// =========================================================

	// the king.
	template <ValidAsset T>
	Asset& addAsset(AssetInfo<T> const& assetInfo);

	template <ValidAsset T>
	void recordAssetFile(
		std::filesystem::path const& path
	);

	// =========================================================
	// Serialising asset meta data..
	template <ValidAsset T>
	friend struct SerialiseMetaDataFunctor;

	template <ValidAsset T>
	friend struct SerialiseAssetFunctor;

	template <ValidAsset T>
	void serialiseAssetMetaData(T const& asset);
	void serialiseAssetMetaData(Asset const& asset, std::ofstream& metaDataFile);

private:
	// This is the callback when assets file are added
	void OnAssetContentAddedCallback(std::string abspath);
	// This is the callback when the assets files are modified/renamed
	void OnAssetContentModifiedCallback(AssetID assetId);
	// This is the callback when any asset file is deleted
	void OnAssetContentDeletedCallback(AssetID assetId);

	std::string GetRunTimeDirectory();

public:
	// Thread pool to manage loading in another thread.
	BS::thread_pool<BS::tp::none> threadPool;

private:

	// main container containing all assets.
	std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;

	// maps filepath to asset id.
	std::unordered_map<std::string, AssetID> filepathToAssetId;

	// groups all assets based on their type.
	std::unordered_map<AssetTypeID, std::vector<AssetID>> assetsByType;
	
	// associates an asset id with the corresponding asset type.
	std::unordered_map<AssetID, AssetTypeID> assetIdToType;

	// -- welcome to template metaprogramming black magic. --
	// associates an asset id with it's corresponding serialising function, containining the original asset type.	
	std::unordered_map<AssetID, std::unique_ptr<SerialiseMetaData>> serialiseMetaDataFunctors;
	//std::unordered_map<AssetID, std::unique_ptr<SerialiseAsset>> serialiseAssetFunctors;

	// when an asset has finished loading, and requires the asset manager to do some post work,
	// the asset will provide a callback here.
	// the asset manager then checks every game frame if there is any callback request.

#endif

public:
	std::unordered_map<FolderID, Folder> const& getDirectories() const;
	std::vector<FolderID> const& getRootDirectories() const;

	void update();

	// as this function may be invoked from different threads, we need to control access to
	// the callback queue.
	void submitCallback(std::function<void()> callback);

private:
	void parseIntermediaryAssetFile(AssetFilePath const& path);

	void recordFolder(
		FolderID folderId,
		std::filesystem::path const& path
	);

	template <ValidAsset T>
	void compileIntermediaryFile(AssetInfo<T> descriptor);

	// =========================================================
	// Parsing meta data file associated with the asset.
	// Each asset will contain a generic metadata (id and name)
	// Each specific type asset has additional metadata as well.
	// =========================================================

	template <ValidAsset T>
	void loadAllDescriptorFiles();

	template <ValidAsset T>
	void createResourceFile(AssetInfo<T> descriptor);

private:
	ResourceManager& resourceManager;
	Engine& engine;

	// The AssetDirectoryWatcher will keep track of the assets directory in a seperate thread
	AssetDirectoryWatcher directoryWatcher;
	
	// records all loaded intermediary assets mapped to it's corresponding descriptor and resource filepath.
	std::unordered_map<AssetFilePath, Filepaths> intermediaryAssetsToFilepaths;

	// Folder metadata (for tree traversal)
	std::unordered_map<FolderID, Folder> directories;
	std::vector<FolderID> rootDirectories;
	std::unordered_map<std::string, FolderID> folderNameToId;

	// the asset manager stores a queue of callbacks that other threads could submit to
	// this ensures thread safety.
	std::queue<std::function<void()>> callbacks;
	std::mutex queueCallbackMutex;	// protects the callback queue.
};

#include "AssetManager/assetMananger.ipp"
#include "AssetManager/assetFunctor.ipp"
#pragma once
#include "BS_thread_pool.hpp"

#include <memory>
#include <unordered_map>
#include <optional>
#include <functional>
#include <queue>
#include <mutex>
#include <atomic>

#include "resource.h"

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

	template <ValidResource T>
	struct AssetQuery {
		// Raw pointer representing non-owning, potentially null resource.
		T* asset;
		QueryResult result;
	};

	struct Descriptor {
		DescriptorFilePath descriptorFilepath;
		BasicAssetInfo descriptor;
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
	// =========================================================
	// Serialising asset meta data..
	template <ValidResource T>
	friend struct SerialiseMetaDataFunctor;

	template <ValidResource T>
	friend struct SerialiseAssetFunctor;

	template <ValidResource T>
	void serialiseAssetMetaData(T const& asset);
	void serialiseAssetMetaData(Asset const& asset, std::ofstream& metaDataFile);
#endif

public:
	std::unordered_map<FolderID, Folder> const& getDirectories() const;
	std::vector<FolderID> const& getRootDirectories() const;

	std::string const& getName(ResourceID id) const;
	AssetFilePath const& getFilepath(ResourceID id) const;
	Descriptor const& getDescriptor(AssetFilePath assetFilePath) const;

	void update();

	// as this function may be invoked from different threads, we need to control access to
	// the callback queue.
	void submitCallback(std::function<void()> callback);

private:
	ResourceID parseIntermediaryAssetFile(AssetFilePath const& path);

	void recordFolder(
		FolderID folderId,
		std::filesystem::path const& path
	);

	template <ValidResource T>
	void compileIntermediaryFile(AssetInfo<T> descriptor);

	// =========================================================
	// Parsing meta data file associated with the asset.
	// Each asset will contain a generic metadata (id and name)
	// Each specific type asset has additional metadata as well.
	// =========================================================

	template<ValidResource ...T>
	void loadAllDescriptorFiles();

	template <ValidResource T>
	ResourceID createResourceFile(AssetInfo<T> descriptor);

private:
	ResourceManager& resourceManager;
	Engine& engine;

	// The AssetDirectoryWatcher will keep track of the assets directory in a seperate thread
	AssetDirectoryWatcher directoryWatcher;
	
	// records all loaded intermediary assets mapped to it's corresponding descriptor.
	std::unordered_map<AssetFilePath, Descriptor> intermediaryAssetsToDescriptor;

	// records metadata for each resource id. these metadata are only used in the editor, like interacting with the
	// the intermediary asset & getting asset name.
	// also acts as a record of all resource id with corresponding descriptor.
	std::unordered_map<ResourceID, BasicAssetInfo> assetToDescriptor;

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
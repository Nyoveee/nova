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
class Editor;

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
	void update();

	// as this function may be invoked from different threads, we need to control access to
	// the callback queue.
	void submitCallback(std::function<void()> callback);

	// Serialising asset meta data..
	template <ValidResource T>
	friend struct SerialiseDescriptorFunctor;

	template <ValidResource T>
	void serialiseDescriptor(ResourceID id);

	bool renameFile(ResourceID id, std::string const& newFileStem);

public:
	// Getters..
	std::unordered_map<FolderID, Folder> const& getDirectories()									const;
	std::string const*							getName(ResourceID id)								const;
	AssetFilePath const*						getFilepath(ResourceID id)							const;
	BasicAssetInfo*								getDescriptor(ResourceID id);
	ResourceID									getResourceID(AssetFilePath const& assetFilePath)	const;

private:
	friend class AssetDirectoryWatcher;
	
	void onAssetAddition(AssetFilePath const& assetFilePath);
	void onAssetModification(ResourceID id, AssetFilePath const& assetFilePath);
	void onFolderModification(std::filesystem::path const& folderPath);

	void onAssetDeletion(ResourceID id);

private:
	void processAssetFilePath(AssetFilePath const& path);

	// creates a new descriptor file and new resource file given the asset file path.
	ResourceID parseIntermediaryAssetFile(AssetFilePath const& path);

	template <ValidResource T>
	void compileIntermediaryFile(AssetInfo<T> const& descriptor);

	template<ValidResource ...T>
	void loadAllDescriptorFiles();

	// creates the resource file given a descriptor via the compiler.
	template <ValidResource T>
	ResourceID createResourceFile(AssetInfo<T> descriptor);

	void recordFolder(FolderID folderId, std::filesystem::path const& path);

private:
	ResourceManager& resourceManager;
	Engine& engine;

	// indicator that says whether the asset manager has fully initialised.
	std::atomic<bool> hasInitialised;

	// The AssetDirectoryWatcher will keep track of the assets directory in a seperate thread
	AssetDirectoryWatcher directoryWatcher;
	
	// records all loaded intermediary assets mapped to it's corresponding descriptor.
	std::unordered_map<AssetFilePath, Descriptor> intermediaryAssetsToDescriptor;

	// records metadata for each resource id. these metadata are only used in the editor, like interacting with the
	// the intermediary asset & getting asset name.
	// also acts as a record of all resource id with corresponding descriptor.
	std::unordered_map<ResourceID, std::unique_ptr<BasicAssetInfo>> assetToDescriptor;

	// for each asset, contains a functor that helps serialise descriptor.
	std::unordered_map<ResourceID, std::unique_ptr<SerialiseDescriptor>> serialiseDescriptorFunctors;

	// map each resource to the original resource type.
	std::unordered_map<ResourceID, ResourceTypeID> resourceToType;

	// Folder metadata (for tree traversal)
	std::unordered_map<FolderID, Folder> directories;
	std::unordered_map<std::string, FolderID> folderPathToId;
	FolderID folderId;

	// the asset manager stores a queue of callbacks that other threads could submit to
	// this ensures thread safety.
	std::queue<std::function<void()>> callbacks;
	std::mutex queueCallbackMutex;	// protects the callback queue.
};

#include "AssetManager/assetMananger.ipp"
#include "AssetManager/assetFunctor.ipp"
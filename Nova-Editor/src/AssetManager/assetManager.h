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

class CubeMap;

class AssetManager {
public:
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
	// reload all recorded assets, descriptors, etc..
	void reload();

	// handle queued operations..
	void update();

	// as this function may be invoked from different threads, we need to control access to
	// the callback queue.
	void submitCallback(std::function<void()> callback);

	// Serialising asset meta data..
	template <ValidResource T>
	friend struct SerialiseDescriptorFunctor;

	// the non templated version is able to call the correct templated function via serialisation functors.
	// that way the call site does not have to know the original type of the resource to call this function.
	void serialiseDescriptor(ResourceID id);

	// use this version if you know what the type is.
	template <ValidResource T>
	void serializeDescriptor(ResourceID id);

	// private function renamefile has the ability to move asset to a new folder.
	// we provide a public facing function that does not allow that. if there is a need to move an asset, 
	// use moveAssetToFolder.
	bool renameFile(ResourceID id, std::string const& newFileName);

	// moves a given asset to another folder..
	bool moveAssetToFolder(ResourceID resourceId, FolderID destinationFolder);

	// Removes this resource from the entry.. might be added back later from recompilation..
	void removeResource(ResourceID id);

	// Deletes the corresponding asset + resource file.
	void deleteAsset(ResourceID id);

	// creates the resource file given a descriptor via the compiler.
	template <ValidResource T>
	ResourceID createResourceFile(AssetInfo<T> descriptor);

	// certain resources are modified in editor runtime
	// we want to serialise this back to an asset. (will call serialiseResource<T> for all resources)
	void serialiseResources();

	// serialises one specific resource.
	template <ValidResource T>
	void serialiseResource(ResourceID id);

	void serialiseCubeMap(CubeMap const& cubeMap);

public:
	// Getters..
	std::unordered_map<FolderID, Folder> const& getDirectories()									const;
	FolderID									getParentFolder(ResourceID id)						const;
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

	// renames a given asset file given a new file stem. (preserves extension)
	// if there is a provided parent folder, rename file will move that asset to that new folder.
	// calling function should provide the descriptor.
	bool renameFile(std::unique_ptr<BasicAssetInfo> const& descriptor, std::string const& newFileName, FolderID parentFolder);

private:
	void processAssetFilePath(AssetFilePath const& path);

	// creates a new descriptor file and new resource file given the asset file path.
	ResourceID parseIntermediaryAssetFile(AssetFilePath const& path);

	template<ValidResource ...T>
	void loadAllDescriptorFiles();

	template <ValidResource T>
	bool compileIntermediaryFile(AssetInfo<T> const& descriptor);

	void recordFolder(FolderID folderId, std::filesystem::path const& path);

	// we use our cache to check if an asset has changed..
	template <ValidResource T>
	bool hasAssetChanged(AssetInfo<T> const& descriptor) const;

	// update our cache file to store the latest write time..
	template <ValidResource T>
	void updateAssetCache(AssetInfo<T> const& descriptor) const;

	template <ValidResource T>
	void loadSystemResourceDescriptor(std::unordered_map<ResourceID, ResourceFilePath> const& systemResources);

	template <ValidResource T>
	void serializeAllResources();

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
	
	// we keep track of system resources because we dont want to serialise them in the end..
	std::unordered_set<ResourceID> systemResourcesId;

	// for each asset, contains a functor that helps serialise descriptor.
	std::unordered_map<ResourceID, std::unique_ptr<SerialiseDescriptor>> serialiseDescriptorFunctors;

	// map each resource to the original resource type.
	std::unordered_map<ResourceID, ResourceTypeID> resourceToType;

	// Folder metadata (for tree traversal)
	std::unordered_map<FolderID, Folder> directories;
	std::unordered_map<ResourceID, FolderID> assetToDirectories;
	std::unordered_map<std::string, FolderID> folderPathToId;
	FolderID folderId;

	// the asset manager stores a queue of callbacks that other threads could submit to
	// this ensures thread safety.
	std::queue<std::function<void()>> callbacks;
	std::mutex queueCallbackMutex;	// protects the callback queue.
};

#include "AssetManager/assetMananger.ipp"
#include "AssetManager/assetFunctor.ipp"
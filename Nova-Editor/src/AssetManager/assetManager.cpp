#include <filesystem>
#include <fstream>
#include <chrono>

#include "AssetManager.h"
#include "FileWatch.hpp"
#include "Logger.h"

#include "cubemap.h"
#include "Engine/engine.h"

#include "Material.h"

#define RecordAssetSubdirectory(AssetType) \
	subAssetDirectories.insert({ Family::id<AssetType>(), descriptorDirectory / #AssetType })

#define LoadAllDescriptorFiles(AssetType) \
	loadAllDescriptorFiles<AssetType>(subAssetDirectories[Family::id<AssetType>()])

namespace {
	// we usually don't want to increment ids
	// but in this case we want so we are explicit about it.
	void incrementFolderId(FolderID& folderId) {
		folderId = std::size_t{ folderId } + 1;
	}
}

AssetManager::AssetManager(ResourceManager& resourceManager, Engine& engine) :
	resourceManager		{ resourceManager },
	engine				{ engine },
	directoryWatcher	{ *this, resourceManager, engine },
	hasInitialised		{},
	folderId			{}
{
	reload();

	// By now everything should be serialized, compile the entity scripts
	engine.scriptingAPIManager.compileScriptAssembly();
}

AssetManager::~AssetManager() {
#if 0
	// let's serialise the asset meta data of all our stored info.
	for (auto&& [id, serialiseFunctorPtr] : serialiseDescriptorFunctors) {
		assert(serialiseFunctorPtr && "Should never be nullptr");

		// serialise the descriptor file for this given asset id.
		serialiseFunctorPtr->operator()(id, *this);
	}
#endif

	// Asset manager serialises all of the resources that is modified directly in the end.
	serialiseResources();
}

void AssetManager::submitCallback(std::function<void()> callback) {
	std::lock_guard lock{ queueCallbackMutex };
	callbacks.push(std::move(callback));
}

void AssetManager::reload() {
	// ========================================
	// 0. Clear all containers, reset all variables..
	// ========================================
	resourceManager.reload();

	hasInitialised = false;
	intermediaryAssetsToDescriptor.clear();
	assetToDescriptor.clear();
	directories.clear();
	systemResourcesId.clear();
	serialiseDescriptorFunctors.clear();
	resourceToType.clear();
	folderPathToId.clear();
	assetToDirectories.clear();
	callbacks = std::queue<std::function<void()>>{};

	// record the root asset directory.
	directories[ASSET_FOLDER] = Folder{
		ASSET_FOLDER,
		ASSET_FOLDER,
		{},
		{},
		"Assets",
		std::filesystem::relative(AssetIO::assetDirectory, AssetIO::assetDirectory)
	};

	folderPathToId[AssetIO::assetDirectory.string()] = ASSET_FOLDER;

	// ========================================
	// 1.1 Check if the descriptor directory exist, and the respective descriptor subdirectories.
	// 1.2 Check if the asset cache directory exist, and the respective asset cache subdirectories.
	// ========================================

	// 1. Checking if the main descriptor directory exist.
	if (!std::filesystem::exists(AssetIO::descriptorDirectory)) {
		std::filesystem::create_directory(AssetIO::descriptorDirectory);
	}

	// Checking if the sub directories exist..
	for (auto&& [_, subDescriptorDirectory] : AssetIO::subDescriptorDirectories) {
		if (!std::filesystem::exists(subDescriptorDirectory)) {
			std::filesystem::create_directory(subDescriptorDirectory);
		}
	}

	// 2. Checking if the asset cache directory exist..
	if (!std::filesystem::exists(AssetIO::assetCacheDirectory)) {
		std::filesystem::create_directory(AssetIO::assetCacheDirectory);
	}

	// Checking if the sub asset cache directory exist..
	for (auto&& [_, subAssetCacheDirectory] : AssetIO::subAssetCacheDirectories) {
		if (!std::filesystem::exists(subAssetCacheDirectory)) {
			std::filesystem::create_directory(subAssetCacheDirectory);
		}
	}

	// ========================================
	// 1.3 Check if the resource directory exist, and the respective sub assets folder exist.
	// ========================================

	// Checking if the main resource directory exist.
	if (!std::filesystem::exists(AssetIO::resourceDirectory)) {
		std::filesystem::create_directory(AssetIO::resourceDirectory);
	}

	// Checking if the sub directories exist..
	for (auto&& [_, subResourceDirectory] : AssetIO::subResourceDirectories) {
		if (!std::filesystem::exists(subResourceDirectory)) {
			std::filesystem::create_directory(subResourceDirectory);
		}
	}

	// ========================================
	// 2. We check if there's new intermediary assets the resource manager has not keep tracked off.
	// Our descriptors act as the source of truth.
	// With descriptors, we verify if every intermediary file has it's corresponding resources asset compiled.
	// If not, we compile the intermediary asset to it's specific resource file and load it into the resources manager.
	// ========================================
	Logger::debug("===========================");
	Logger::debug("Loading all descriptor files..");
	Logger::debug("===========================\n");

	loadAllDescriptorFiles<ALL_RESOURCES>();

	// ========================================
	// 3. We need to check if every intermediary assets has a corresponding descriptor pointing to it.
	// When we add new intermediary assets to the Assets folder, we need to generate it's corresponding descriptor.
	// 
	// When we iterate through all the intermediate assets, we also keep track of the directory it is in to record
	// the entire directory structure for asset viewer UI.
	// ========================================
	Logger::debug("===========================");
	Logger::debug("Verifying every intermediary assets..");
	Logger::debug("===========================\n");

	for (const auto& entry : std::filesystem::recursive_directory_iterator{ AssetIO::assetDirectory }) {
		processAssetFilePath(entry.path());
	}

	// ========================================
	// 4. We do some housekeeping, and find any outdated resource files with no corresponding descriptors.
	// ========================================
	for (const auto& entry : std::filesystem::recursive_directory_iterator{ AssetIO::resourceDirectory }) {
		std::filesystem::path resourcePath = entry.path();

		if (!entry.is_regular_file()) {
			continue;
		}

		try {
			ResourceID id = std::stoull(std::filesystem::path{ resourcePath }.stem().string());
			auto iterator = assetToDescriptor.find(id);

			if (iterator == assetToDescriptor.end()) {
				Logger::debug("Found dangling resource file {} with no corresponding descriptor file, removing it..", resourcePath.string());
				std::filesystem::remove(resourcePath);
				resourceManager.removeResource(id);
			}
		}
		catch (std::exception const&) {
			Logger::debug("Failed to get resource id {} from file, removing it..", resourcePath.string());
		}
	}

	// ========================================
	// 5. We load descriptors for system resources, most importantly name.
	// ========================================
	Logger::debug("===========================");
	Logger::debug("Loading system resources descriptors..");
	Logger::debug("===========================\n");

	loadSystemResourceDescriptor<Model>(AssetIO::systemModelResources);
	loadSystemResourceDescriptor<CustomShader>(AssetIO::systemShaderResources);
	loadSystemResourceDescriptor<Material>(AssetIO::systemMaterialResources);
	loadSystemResourceDescriptor<Texture>(AssetIO::systemTextureResources);

	hasInitialised = true;
}

void AssetManager::update() {
	std::lock_guard lock{ queueCallbackMutex };
	
	while (callbacks.size()) {
		std::function<void()> callback = callbacks.front();
		callbacks.pop();
		callback();
	}
}

ResourceID AssetManager::parseIntermediaryAssetFile(AssetFilePath const& assetFilePath) {
	// time to check if file has valid file extension.
	std::string fileExtension = std::filesystem::path{ assetFilePath }.extension().string();

	// lambda creates descriptor file, creates the resource file, and loads it to the resource manager.
	auto initialiseResourceFile = [&]<typename T>() {
		auto assetInfo = AssetIO::createDescriptorFile<T>(assetFilePath);
		ResourceID id = createResourceFile<T>(assetInfo);
		return id;
	};

	if (fileExtension == ".fbx" || fileExtension == ".obj") {
		return initialiseResourceFile.template operator()<Model>();
	}
	else if (fileExtension == ".png" || fileExtension == ".jpg") {
		return initialiseResourceFile.template operator()<Texture>();
	}
	else if (fileExtension == ".hdr") {
		return initialiseResourceFile.template operator()<CubeMap>();
	}
	else if (fileExtension == ".cs") {
		return initialiseResourceFile.template operator()<ScriptAsset>();
	}
	else if (fileExtension == ".wav") {
		return initialiseResourceFile.template operator()<Audio>();
	}
	else if (fileExtension == ".scene") {
		return initialiseResourceFile.template operator()<Scene>();
	}
	else if (fileExtension == ".navmesh") {
		return initialiseResourceFile.template operator()<NavMesh>();
	}
	else if (fileExtension == ".controller") {
		return initialiseResourceFile.template operator()<Controller>();
	}
	else if (fileExtension == ".shader") {
		return initialiseResourceFile.template operator()<CustomShader>();
	}
	else if (fileExtension == ".material") {
		return initialiseResourceFile.template operator()<Material>();
	}
	else if (fileExtension == ".ttf") {
		return initialiseResourceFile.template operator()<Font>();
	}
	else if (fileExtension == ".prefab") {
		return initialiseResourceFile.template operator()<Prefab> ();
	}
	else if (fileExtension == ".sequencer") {
		return initialiseResourceFile.template operator()<Sequencer>();
	}
	else if (fileExtension == ".mpeg") {
		return initialiseResourceFile.template operator()<Video>();
	}
	else {
		Logger::warn("Unsupported file type of: {} has been found.", assetFilePath.string);
		return INVALID_RESOURCE_ID;
	}
}

void AssetManager::recordFolder(FolderID id, std::filesystem::path const& path) {
	if (id == ASSET_FOLDER) {
		return;
	}

	// Get parent directory.
	std::filesystem::path parentPath = path.parent_path();

	FolderID parentFolderId;

	// This directory is at the root asset folder -> no parent.
	if (parentPath == AssetIO::assetDirectory) {
		parentFolderId = ASSET_FOLDER;
	}
	else {
		parentFolderId = folderPathToId[parentPath.string()];
	}

	auto iterator = directories.find(parentFolderId);

	if (iterator != directories.end()) {
		iterator->second.childDirectories.push_back(id);
	}
	else {
		assert(false && "Depth first search guarantees that the parent folder should have been recorded.");
	}

	folderPathToId[path.string()] = id;

	directories[folderId] = Folder{
		id,
		parentFolderId,
		{},
		{},
		path.filename().string(),
		std::filesystem::relative(path, AssetIO::assetDirectory)
	};
}

bool AssetManager::renameFile(std::unique_ptr<BasicAssetInfo> const& descriptor, std::string const& newFileName, FolderID parentFolder) {
	// Determine the appropriate full new file path.
	std::filesystem::path oldFullFilePath { descriptor->filepath };

	std::filesystem::path parentPath = [&]() {
		if (parentFolder == NO_FOLDER) {
			// no specified parent folder, we rename it based on it's current parent filepath..
			return std::filesystem::path{ descriptor->filepath }.parent_path();
		}
		else {
			// get the specified parent folder's filepath.
			Folder const& folder = directories.at(parentFolder);
			return AssetIO::assetDirectory / folder.relativePath;
		}
	}().lexically_normal();
	
	// uses the original file name if newFileStem is empty.
	std::filesystem::path newFullFilePath = newFileName.empty()
		? std::move(parentPath) / std::filesystem::path{ descriptor->filepath }.stem()
		: std::move(parentPath) / newFileName;

	newFullFilePath.replace_extension(oldFullFilePath.extension());

	if (std::filesystem::exists(newFullFilePath)) {
		Logger::error("Attempt to rename file failed. New file path already exist.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::rename(oldFullFilePath, newFullFilePath, errorCode);

	if (errorCode) {
		Logger::error(
			"Attempt to rename file failed w/ error code {}"
			"\nAttempt to rename from: {} to {}.", errorCode.value(), oldFullFilePath.string(), newFullFilePath.string()
		);

		return false;
	}

	auto nodeHandle = intermediaryAssetsToDescriptor.extract(descriptor->filepath);

	if (nodeHandle) {
		nodeHandle.key() = newFullFilePath;
		intermediaryAssetsToDescriptor.insert(std::move(nodeHandle));
	}
	else {
		assert(false && "Intermediary filepath not recorded. Invariant broken.");
	}

	descriptor->name = newFileName;
	descriptor->filepath = newFullFilePath;

	Logger::debug("Asset filepath rename successful.");

	return true;
}

std::string const* AssetManager::getName(ResourceID id) const {
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		return nullptr;
	}

	auto&& [_, descriptor] = *iterator;
	assert(descriptor && "This ptr should never be null.");
	return &descriptor->name;
}

AssetFilePath const* AssetManager::getFilepath(ResourceID id) const {
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		return nullptr;
	}

	auto&& [_, descriptor] = *iterator;
	assert(descriptor && "This ptr should never be null.");
	return &descriptor->filepath;
}

BasicAssetInfo* AssetManager::getDescriptor(ResourceID id) {
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		return nullptr;
	}

	auto&& [_, descriptor] = *iterator;
	return descriptor.get();
}

ResourceID AssetManager::getResourceID(AssetFilePath const& assetFilePath) const {
	auto iterator = intermediaryAssetsToDescriptor.find(assetFilePath);

	if (iterator == intermediaryAssetsToDescriptor.end()) {
		return INVALID_RESOURCE_ID;
	}

	auto&& [_, descriptor] = *iterator;
	return descriptor.descriptor.id;
}

void AssetManager::onAssetAddition(AssetFilePath const& assetFilePath) {
	auto iterator = intermediaryAssetsToDescriptor.find(assetFilePath);

	// !! new intermediary asset.
	if (iterator == std::end(intermediaryAssetsToDescriptor)) {
		processAssetFilePath(assetFilePath);
	}
}

void AssetManager::onAssetModification(ResourceID id, AssetFilePath const& assetFilePath) {
	Logger::debug("Asset watcher notices that asset {} has changed, recompiling it's corresponding resource file..\n", assetFilePath.string);

	auto recreateResourceFile = [&]<ValidResource ...T>(ResourceID id) {
		([&] {
			if (Family::id<T>() == resourceToType[id]) {
				auto iterator = assetToDescriptor.find(id);

				if (iterator != assetToDescriptor.end()) {
					AssetInfo<T>* descriptor = static_cast<AssetInfo<T>*>(iterator->second.get());
					if (!descriptor) {
						Logger::error("This should never be nullptr.");
						return;
					}

					resourceManager.removeResource(id);
					createResourceFile<T>(*descriptor);
				}
				else {
					Logger::error("Failed to retrieve descriptor when attempting to recompile asset.");
				}
			}
		}(), ...);
	};

	recreateResourceFile.template operator()<ALL_RESOURCES>(id);
}

void AssetManager::onFolderModification(std::filesystem::path const& folderPath) {
	(void) folderPath;
#if 0
	// Get original folder id corresponding to the folder path.
	auto iterator = folderPathToId.find(folderPath.string());

	if (iterator == folderPathToId.end()) {
		Logger::error("Unknown folder has been modified?");
		return;
	}

	auto&& [_, folderId] = *iterator;
	
	// Edit folder metadata..
	Folder& folder = directories.at(folderId);
	folder.name = folderPath.stem().string();
	folder.relativePath = std::filesystem::relative(folderPath, AssetIO::assetDirectory);

	// Edit folderPath 
	auto nodeHandle = folderPathToId.extract(folderPath.string());
	nodeHandle.key() = folderPath;
#endif
}

void AssetManager::onAssetDeletion(ResourceID id) {
	(void) id;
}

void AssetManager::processAssetFilePath(AssetFilePath const& assetPath) {
	if (directoryWatcher.IsPathHidden(std::filesystem::path{ assetPath })) {
		return;
	}

	else if (std::filesystem::is_directory(std::filesystem::path{ assetPath })) {
		recordFolder(folderId, std::filesystem::path{ assetPath });
		incrementFolderId(folderId);
		return;
	}

	else if (!std::filesystem::is_regular_file(std::filesystem::path{ assetPath })) {
		return;
	}

	// ------------------------------------------
	// Attempt to retrieve resource id from asset file path.
	// ------------------------------------------
	ResourceID resourceId = INVALID_RESOURCE_ID;
	auto iterator = intermediaryAssetsToDescriptor.find(assetPath);

	// !! intermediary asset not recorded (missing corresponding descriptor and resource file.)
	if (iterator == std::end(intermediaryAssetsToDescriptor)) {
		Logger::debug("Intermediary asset {} has missing file descriptor and resource file. Creating one..", assetPath.string);
		Logger::debug("");

		// we need to generate the corresponding descriptor file for this intermediary file.
		resourceId = parseIntermediaryAssetFile(assetPath);
	}
	// intermediary asset recorded, retrieve id..
	else {
		resourceId = iterator->second.descriptor.id;
	}

	if (resourceId == INVALID_RESOURCE_ID) {
		return;
	}

	// ------------------------------------------
	// Save resource entry in the parent folder.
	// ------------------------------------------

	std::filesystem::path parentPath = std::filesystem::path{ assetPath }.parent_path();

	auto folderIterator = folderPathToId.find(parentPath.string());

	if (folderIterator == std::end(folderPathToId)) {
		Logger::error("Attempting to add asset to a non existing parent folder?");
	}
	else {
		auto&& [_, parentFolderId] = *folderIterator;
		directories[parentFolderId].assets.insert(resourceId);
		assetToDirectories[resourceId] = parentFolderId;
	}
}

void AssetManager::serialiseResources() {
	serializeAllResources<Controller>();
	serializeAllResources<Material>();
	serializeAllResources<CustomShader>();
	serializeAllResources<Sequencer>();
}

bool AssetManager::moveAssetToFolder(ResourceID resourceId, FolderID destinationFolderId) {
	auto iterator = assetToDescriptor.find(resourceId);

	if (iterator == assetToDescriptor.end()) {
		Logger::error("Attempt to move file failed. Unable to find corresponding descriptor.");
		return false;
	}

	auto&& [_, descriptor] = *iterator;

	// We attempt to actually move the underlying asset first.. (because it may fail)..
	if (!renameFile(descriptor, descriptor->name, destinationFolderId)) {
		return false;
	}

	// update the directories metadata..
	// we first update the original folder..
	auto originalFolderIterator = assetToDirectories.find(resourceId);
	assert(originalFolderIterator != assetToDirectories.end() && "Since resourceId exist in asset to descriptor, it must be valid. Invariant broken.");
	
	auto&& [__, originalFolderId] = *originalFolderIterator;
	
	Folder& originalFolder = directories.at(originalFolderId);
	originalFolder.assets.erase(resourceId);
	
	// update the map..
	originalFolderId = destinationFolderId;

	// update the new folder..
	Folder& destinationFolder = directories.at(destinationFolderId);
	destinationFolder.assets.insert(resourceId);

	return true;
}

std::unordered_map<FolderID, Folder> const& AssetManager::getDirectories() const {
	return directories;
}

FolderID AssetManager::getParentFolder(ResourceID id) const {
	auto iterator = assetToDirectories.find(id);
	
	if (iterator == assetToDirectories.end()) {
		return NO_FOLDER;
	}

	return iterator->second;
}

bool AssetManager::renameFile(ResourceID id, std::string const& newFileName) {
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		Logger::error("Attempt to rename file failed. Unable to find corresponding descriptor.");
		return false;
	}

	auto&& [_, descriptor] = *iterator;

	return renameFile(descriptor, newFileName, NO_FOLDER);
}

void AssetManager::removeResource(ResourceID id) {
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		Logger::warn("Removing an invalid resource?");
		return;
	}

	auto&& [_, assetInfoPtr] = *iterator;

	intermediaryAssetsToDescriptor.erase(assetInfoPtr->filepath);
	serialiseDescriptorFunctors.erase(id);
	resourceToType.erase(id);
	assetToDescriptor.erase(iterator);
	assetToDirectories.erase(id);
}

void AssetManager::deleteAsset([[maybe_unused]] ResourceID id) {
	// via the descriptor, find the original asset filepath..
	auto iterator = assetToDescriptor.find(id);

	if (iterator == assetToDescriptor.end()) {
		Logger::warn("Deleting an invalid resource?");
		return;
	}

	// Delete the asset file..
	auto&& [_, assetInfoPtr] = *iterator;
 	std::filesystem::remove(assetInfoPtr->filepath);

	// Remove any record of this asset in memory..
	removeResource(id);
	resourceManager.removeResource(id);
}

void AssetManager::serialiseDescriptor(ResourceID id) {
	auto iterator = serialiseDescriptorFunctors.find(id);
	
	if (iterator == serialiseDescriptorFunctors.end()) {
		Logger::error("Failed to serialize descriptor for resource id {}!", static_cast<std::size_t>(id));
		return;
	}

	iterator->second->operator()(id, *this);
}
#include <filesystem>
#include <fstream>
#include <chrono>

#include "AssetManager.h"
#include "FileWatch.hpp"
#include "Logger.h"

#include "cubemap.h"

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
	directoryWatcher	{ *this, resourceManager, engine, Descriptor::assetDirectory }
{
	// ========================================
	// 1. Check if the descriptor directory exist, and the respective asset subdirectories.
	// ========================================

	// Checking if the main descriptor directory exist.
	if (!std::filesystem::exists(Descriptor::descriptorDirectory)) {
		std::filesystem::create_directory(Descriptor::descriptorDirectory);
	}

	// Checking if the sub directories exist..
	for (auto&& [_, subDescriptorDirectory] : Descriptor::subDescriptorDirectories) {
		if (!std::filesystem::exists(subDescriptorDirectory)) {
			std::filesystem::create_directory(subDescriptorDirectory);
		}
	}

	// ========================================
	// 2. We check if there's new intermediary assets the resource manager has not keep tracked off.
	// Our descriptors act as the source of truth.
	// With descriptors, we verify if every intermediary file has it's corresponding resources asset compiled.
	// If not, we compile the intermediary asset to it's specific resource file and load it into the resources manager.
	// ========================================
	Logger::info("===========================");
	Logger::info("Loading all descriptor files..");
	Logger::info("===========================\n");

	loadAllDescriptorFiles<Texture>();
	loadAllDescriptorFiles<Model>();
	loadAllDescriptorFiles<CubeMap>();
	loadAllDescriptorFiles<ScriptAsset>();
	loadAllDescriptorFiles<Audio>();

	// ========================================
	// 3. We need to check if every intermediary assets has a corresponding descriptor pointing to it.
	// When we add new intermediary assets to the Assets folder, we need to generate it's corresponding descriptor.
	// 
	// When we iterate through all the intermediate assets, we also keep track of the directory it is in to record
	// the entire directory structure for asset viewer UI.
	// ========================================
	Logger::info("===========================");
	Logger::info("Verifying every intermediary assets..");
	Logger::info("===========================\n");

	FolderID folderId{ 0 };

	for (const auto& entry : std::filesystem::recursive_directory_iterator{ Descriptor::assetDirectory }) {
		std::filesystem::path assetPath = entry.path();

		if (directoryWatcher.IsPathHidden(assetPath)) {
			continue;
		}

		if (entry.is_directory()) {
			recordFolder(folderId, assetPath);
			incrementFolderId(folderId);
			continue;
		}

		else if (!entry.is_regular_file()) {
			continue;
		}

		// intermediary asset not recorded (missing corresponding descriptor).
		auto iterator = intermediaryAssetsToFilepaths.find(assetPath);
		
		if (iterator == std::end(intermediaryAssetsToFilepaths)) {
			Logger::info("Intermediary asset {} has missing file descriptor and resource file. Creating one..", assetPath.string());

			// we need to generate the corresponding descriptor file for this intermediary file.
			parseIntermediaryAssetFile(assetPath);
			Logger::info("");
		}

		// ------------------------------------------
		// Save resource entry in the parent folder.
		// ------------------------------------------
		ResourceID resourceId = resourceManager.getResourceID(intermediaryAssetsToFilepaths[assetPath].resourceFilepath);

		// found some intermediary asset file that is not supported.
		if (resourceId == INVALID_ASSET_ID) {
			continue;
		}

		std::filesystem::path parentPath = assetPath.parent_path();

		auto folderIterator = folderNameToId.find(parentPath.string());

		if (folderIterator == std::end(folderNameToId)) {
			Logger::error("Attempting to add asset to a non existing parent folder?");
		}
		else {
			auto&& [_, parentFolderId] = *folderIterator;
			directories[parentFolderId].assets.push_back(resourceId);
		}
		// ------------------------------------------
	}
}

AssetManager::~AssetManager() {
#if 0
	// let's serialise the asset meta data of all our stored info.
	for (auto&& [id, assetPtr] : assets) {
		Asset* asset = assetPtr.get();

		if (!asset) {
			Logger::error("Asset manager is storing an invalid asset!");
			continue;
		}

		auto&& serialiseFunctorPtr = serialiseMetaDataFunctors[id];

		if (!serialiseFunctorPtr) {
			Logger::error("Asset manager failed to record the appropriate serialisation functor!");
			continue;
		}

		// cool syntax? or abomination?
		serialiseFunctorPtr->operator()(*asset, *this);
	}
#endif
}

#if 0


void AssetManager::serialiseAssetMetaData(Asset const& asset, std::ofstream& metaDataFile) {
	metaDataFile << static_cast<std::size_t>(asset.id) << "\n" << asset.name << "\n";
}

void AssetManager::OnAssetContentAddedCallback(std::string abspath)
{
	Logger::info("Called Asset Directory Added, {}", abspath);
}

void AssetManager::OnAssetContentModifiedCallback(AssetID assetId){
	Logger::info("Called Asset Directory Modified, {}", static_cast<std::size_t>(assetId));
}

void AssetManager::OnAssetContentDeletedCallback(AssetID assetId){
	Logger::info("Called Asset Directory Content Deleted, {}", static_cast<std::size_t>(assetId));
}

std::string AssetManager::GetRunTimeDirectory() {
	std::string runtimeDirectory = std::string(MAX_PATH, '\0');
	GetModuleFileNameA(nullptr, runtimeDirectory.data(), MAX_PATH);
	PathRemoveFileSpecA(runtimeDirectory.data());
	runtimeDirectory.resize(std::strlen(runtimeDirectory.data()));
	return runtimeDirectory;
}

#endif

void AssetManager::submitCallback(std::function<void()> callback) {
	std::lock_guard lock{ queueCallbackMutex };
	callbacks.push(std::move(callback));
}

void AssetManager::update() {
	std::lock_guard lock{ queueCallbackMutex };
	
	while (callbacks.size()) {
		std::function<void()> callback = callbacks.front();
		callbacks.pop();
		callback();
	}
}

void AssetManager::parseIntermediaryAssetFile(AssetFilePath const& assetFilePath) {
	// time to check if file has valid file extension.
	std::string fileExtension = std::filesystem::path{ assetFilePath }.extension().string();

	// lambda creates descriptor file, loads it to the resource manager and records it as loaded.
	auto loadDescriptorFile = [&]<typename T>() {
		auto assetInfo = Descriptor::createDescriptorFile<T>(assetFilePath);

		DescriptorFilePath descriptorFilePath = Descriptor::getDescriptorFilename<T>(assetInfo.id);
		ResourceFilePath resourceFilePath = Descriptor::getResourceFilename<T>(assetInfo.id);

		intermediaryAssetsToFilepaths.insert({ assetFilePath, { descriptorFilePath, resourceFilePath } });
		createResourceFile<T>(assetInfo);
	};

	if (fileExtension == ".fbx") {
		loadDescriptorFile.template operator()<Model>();
	}
	else if (fileExtension == ".png" || fileExtension == ".jpg") {
		loadDescriptorFile.template operator()<Texture>();
	}
	else if (fileExtension == ".exr") {
		loadDescriptorFile.template operator()<CubeMap>();
	}
	else if (fileExtension == ".cs") {
		loadDescriptorFile.template operator()<ScriptAsset>();
	}
	else if (fileExtension == ".wav") {
		loadDescriptorFile.template operator()<Audio>();
	}
	else {
		Logger::warn("Unsupported file type of: {} has been found.", assetFilePath.string);
		return;
	}
}

void AssetManager::recordFolder(FolderID folderId, std::filesystem::path const& path) {
	// Get parent directory.
	std::filesystem::path parentPath = path.parent_path();

	FolderID parentFolderId;

	// This directory is at the root asset folder -> no parent.
	if (parentPath == Descriptor::assetDirectory) {
		parentFolderId = NONE;
		rootDirectories.push_back(folderId);
	}
	else {
		parentFolderId = folderNameToId[parentPath.string()];
		directories[parentFolderId].childDirectories.push_back(folderId);
	}

	directories[folderId] = Folder{
		folderId,
		parentFolderId,
		{},
		{},
		path.filename().string(),
		std::filesystem::relative(path, Descriptor::assetDirectory)
	};

	folderNameToId[path.string()] = folderId;
}

std::vector<FolderID> const& AssetManager::getRootDirectories() const {
	return rootDirectories;
}

std::unordered_map<FolderID, Folder> const& AssetManager::getDirectories() const {
	return directories;
}
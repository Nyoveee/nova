#include <filesystem>
#include <fstream>
#include <chrono>

#include "AssetManager.h"
#include "FileWatch.hpp"
#include "Logger.h"

#include "cubemap.h"

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
	assetDirectory		{ std::filesystem::current_path() / "Assets" },
	descriptorDirectory	{ std::filesystem::current_path() / "Descriptors" },
	directoryWatcher	{ *this, resourceManager, engine, std::filesystem::current_path() / "Assets" }

#if 0
	resourceDirectory	{ std::filesystem::current_path() /= "Resources" },
	threadPool			{ static_cast<std::size_t>(std::thread::hardware_concurrency() / 2U - 1U) }, 
#endif
{
	// ========================================
	// 1. Record all the different asset subdirectories into our map.
	// ========================================
	RecordAssetSubdirectory(Texture);
	RecordAssetSubdirectory(Model);
	RecordAssetSubdirectory(CubeMap);
	RecordAssetSubdirectory(ScriptAsset);
	RecordAssetSubdirectory(Audio);

	// ========================================
	// 2. Check if the descriptor directory exist, and the respective asset subdirectories.
	// ========================================

	// Checking if the main descriptor directory exist.
	if (!std::filesystem::exists(descriptorDirectory)) {
		std::filesystem::create_directory(descriptorDirectory);
	}

	// Checking if the sub directories exist..
	for (auto&& [_, subAssetDirectory] : subAssetDirectories) {
		if (!std::filesystem::exists(subAssetDirectory)) {
			std::filesystem::create_directory(subAssetDirectory);
		}
	}

	// ========================================
	// 3. We check if there's new intermediary assets the resource manager has not keep tracked off.
	// Our descriptors act as the source of truth.
	// With descriptors, we verify if every intermediary file has it's corresponding resources asset compiled.
	// If not, we compile the intermediary asset to it's specific resource file and load it into the resources manager.
	// ========================================
	LoadAllDescriptorFiles(Texture);
	LoadAllDescriptorFiles(Model);
	LoadAllDescriptorFiles(CubeMap);
	LoadAllDescriptorFiles(ScriptAsset);
	LoadAllDescriptorFiles(Audio);

	// ========================================
	// 4. We need to check if every intermediary assets has a corresponding descriptor pointing to it.
	// When we add new intermediary assets to the Assets folder, we need to generate it's corresponding descriptor.
	// 
	// When we iterate through all the intermediate assets, we also keep track of the directory it is in to record
	// the entire directory structure for asset viewer UI.
	// ========================================
	FolderID folderId{ 0 };

	for (const auto& entry : std::filesystem::recursive_directory_iterator{ assetDirectory }) {
		std::filesystem::path currentPath = entry.path();

		if (directoryWatcher.IsPathHidden(currentPath)) {
			continue;
		}

		if (entry.is_directory()) {
			recordFolder(folderId, currentPath);
			incrementFolderId(folderId);
		}

		else if (!entry.is_regular_file()) {
			continue;
		}

		if (!loadedIntermediaryAssets.count(currentPath.string())) {
			// we need to generate the corresponding descriptor file for this intermediary file.
			parseIntermediaryAssetFile(currentPath);
		}
		
		// ------------------------------------------
		// Save resource entry in the parent folder.
		// ------------------------------------------
		ResourceID resourceId = resourceManager.getResourceID(currentPath);

		// found some intermediary asset file that is not supported.
		if (resourceId == INVALID_ASSET_ID) {
			continue;
		}

		std::filesystem::path parentPath = currentPath.parent_path();

		auto iterator = folderNameToId.find(parentPath.string());

		if (iterator == std::end(folderNameToId)) {
			Logger::error("Attempting to add asset to a non existing parent folder?");
		}
		else {
			auto&& [_, parentFolderId] = *iterator;
			directories[parentFolderId].assets.push_back(resourceId);
		}
		// ------------------------------------------
	}

#if 0
	// Register callbacks for the watcher
	directoryWatcher.RegisterCallbackAssetContentAdded([&](std::string absPath) { OnAssetContentAddedCallback(absPath); });
	directoryWatcher.RegisterCallbackAssetContentModified([&](ResourceID resourceId) { OnAssetContentModifiedCallback(resourceId); });
	directoryWatcher.RegisterCallbackAssetContentDeleted([&](ResourceID resourceId) { OnAssetContentDeletedCallback(resourceId); });
#endif
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

std::optional<BasicAssetInfo> AssetManager::parseDescriptorFile(std::filesystem::path const& path, std::ifstream& metaDataFile) {
	// Attempt to read corresponding metafile.
	if (!metaDataFile) {
		return std::nullopt;
	}

	std::string line;
	ResourceID resourceId;

	// reads the 1st line.
	std::getline(metaDataFile, line);

	try {
		resourceId = std::stoull(line);
	}
	catch (std::exception const& exception) {
		Logger::warn("Failed to parse metadata file for file {}.\nException: {}", path.string(), exception.what());
		return std::nullopt;
	}

	// reads the 2nd line, name.

	std::string name;
	std::getline(metaDataFile, name);

	// reads the 3rd line, relative filepath.
	std::string relativeFilepath;
	std::getline(metaDataFile, relativeFilepath);
	std::string fullFilepath = std::filesystem::path{ assetDirectory / relativeFilepath }.string();

	Logger::info("Successfully parsed metadata file for {}", path.string());
	return { { resourceId, std::move(name), std::move(fullFilepath) } };
}

BasicAssetInfo AssetManager::createDescriptorFile(std::filesystem::path const& path, std::ofstream& metaDataFile) {
	// calculate relative path to the Assets directory.
	std::filesystem::path relativePath = std::filesystem::relative(path, assetDirectory);

	BasicAssetInfo assetInfo = { generateResourceID(path), path.filename().string(), path.string() };

	if (!metaDataFile) {
		Logger::error("Error creating metadata file for {}", path.string());
		return assetInfo;
	}

	// write to file
	metaDataFile << static_cast<std::size_t>(assetInfo.id) << '\n' << assetInfo.name << '\n' << relativePath.string() << '\n';

	return assetInfo;
}

ResourceID AssetManager::generateResourceID(std::filesystem::path const& path) const {
	// We don't need complex id generation code.
	// We just need to make sure our id is unique everytime a metadata file is generated in the context of our system
	// The easiest way is to utilise the current time + the filepath.

	// average c++ attempting to get time line of code..
	std::size_t time = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string combined = path.string() + "_" + std::to_string(time);
	ResourceID id { std::hash<std::string>{}(combined) };

	return id;
}

void AssetManager::parseIntermediaryAssetFile(std::filesystem::path const& path) {
	// time to check if file has valid file extension.
	std::string fileExtension = path.extension().string();

	// lambda creates descriptor file, loads it to the resource manager and records it as loaded.
	auto loadDescriptorFile = [&]<typename T>() {
		auto assetInfo = createDescriptorFile<T>(path);
		resourceManager.addResourceFile(assetInfo);
		loadedIntermediaryAssets.insert(assetInfo.filepath);
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
		Logger::warn("Unsupported file type of: {} has been found.", path.string());
		return;
	}
}

void AssetManager::recordFolder(FolderID folderId, std::filesystem::path const& path) {
	// Get parent directory.
	std::filesystem::path parentPath = path.parent_path();

	FolderID parentFolderId;

	// This directory is at the root asset folder -> no parent.
	if (parentPath == assetDirectory) {
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
		std::filesystem::relative(path, assetDirectory)
	};

	folderNameToId[path.string()] = folderId;
}

std::vector<FolderID> const& AssetManager::getRootDirectories() const {
	return rootDirectories;
}

std::unordered_map<FolderID, Folder> const& AssetManager::getDirectories() const {
	return directories;
}
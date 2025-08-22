#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <chrono>

#include "AssetManager.h"
#include "Libraries/FileWatch.hpp"

namespace {
	// we usually don't want to increment ids
	// but in this case we want so we are explicit about it.
	void incrementFolderId(FolderID& folderId) {
		folderId = std::size_t{ folderId } + 1;
	}
}

AssetManager::AssetManager() :
	threadPool {}
{
	std::filesystem::path assetDirectory = std::filesystem::current_path();
	assetDirectory /= "Assets";

	FolderID folderId{ 0 };
		
	try {
		for (const auto& entry : std::filesystem::recursive_directory_iterator{ assetDirectory }) {
			std::filesystem::path currentPath = entry.path();
			
			// our own meta file, not an asset.
			if (currentPath.extension() == ".nova_meta") {
				continue;
			}

			if (entry.is_directory()) {
				recordFolder(folderId, currentPath, assetDirectory);
				incrementFolderId(folderId);
			}

			else if (entry.is_regular_file()) {
				parseAssetFile(currentPath);
			}
		}
	}
	catch (const std::filesystem::filesystem_error& ex) {
		spdlog::error("Filesystem error: {}", ex.what());
	}
}

AssetManager::~AssetManager() {
	// let's serialise the asset meta data of all our stored info.
	for (auto&& [id, assetPtr] : assets) {
		Asset* asset = assetPtr.get();

		if (!asset) {
			spdlog::error("Asset manager is storing an invalid asset!");
			continue;
		}

		auto&& serialiseFunctorPtr = serialiseAssetFunctors[id];

		if (!serialiseFunctorPtr) {
			spdlog::error("Asset manager failed to record the appropriate serialisation functor!");
			continue;
		}

		// cool syntax? or abomination?
		serialiseFunctorPtr->operator()(*asset, *this);
	}
}

void AssetManager::update() {
	while (completedLoadingCallback.size()) {
		std::function<void()> callback = completedLoadingCallback.front();
		
		callback();

		completedLoadingCallback.pop();
	}
}

Asset* AssetManager::getAssetInfo(AssetID id) {
	auto iterator = assets.find(id);

	if (iterator == std::end(assets)) {
		return nullptr;
	}

	auto&& [_, asset] = *iterator;
	return asset.get();
}

void AssetManager::recordFolder(
	FolderID folderId,
	std::filesystem::path const& path,
	std::filesystem::path const& assetDirectory
) {
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

void AssetManager::parseAssetFile(std::filesystem::path const& path) {
	// time to check if file has valid file extension.
	std::string fileExtension = path.extension().string();

	if (fileExtension == ".fbx") {
		recordAssetFile<Model>(path);
	}

	else if (fileExtension == ".png" || fileExtension == ".jpg") {
		recordAssetFile<Texture>(path);
	}

	else {
		spdlog::warn("Unsupported file type of: {} has been found.", path.string());
	}
}

AssetID AssetManager::generateAssetID(std::filesystem::path const& path) {
	// We don't need complex id generation code.
	// We just need to make sure our id is unique everytime a metadata file is generated in the context of our system
	// The easiest way is to utilise the current time + the filepath.
	
	AssetID id;

	do {
		// average c++ attempting to get time line of code..
		std::size_t time = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		std::string combined = path.string() + "_" + std::to_string(time);
		id = { std::hash<std::string>{}(combined) };
	}
	// collision, regenerate.
	while (assets.contains(id));

	return id;
}

std::optional<BasicAssetInfo> AssetManager::parseMetaDataFile(std::filesystem::path const& path, std::ifstream& metaDataFile) {
	// Attempt to read corresponding metafile.
	if (!metaDataFile) {
		return std::nullopt;
	}

	std::string line;
	AssetID assetId;

	// reads the 1st line.
	std::getline(metaDataFile, line);

	try {
		assetId = std::stoull(line);
	}
	catch (std::exception const& exception) {
		spdlog::warn("Failed to parse metadata file for file {}.\nException: {}", path.string(), exception.what());
		return std::nullopt;
	}

	// reads the 2nd line.
	std::getline(metaDataFile, line);

	spdlog::info("Successfully parsed metadata file for {}", path.string());
	return {{ assetId, path.string(), line }};
}

BasicAssetInfo AssetManager::createMetaDataFile(std::filesystem::path const& path, std::ofstream& metaDataFile) {
	BasicAssetInfo assetInfo = { generateAssetID(path), path.string(), path.filename().string() };

	if (!metaDataFile) {
		spdlog::error("Error creating metadata file for {}", path.string());
		return assetInfo;
	}

	// write to file
	metaDataFile << static_cast<std::size_t>(assetInfo.id) << "\n" << assetInfo.name << "\n";

	return assetInfo;
}

void AssetManager::serialiseAssetMetaData(Asset const& asset, std::ofstream& metaDataFile) {
	metaDataFile << static_cast<std::size_t>(asset.id) << "\n" << asset.name << "\n";
}

void AssetManager::submitCallback(std::function<void()> callback) {
	std::lock_guard lock{ queueCallbackMutex };

	completedLoadingCallback.push(std::move(callback));
}

std::unordered_map<FolderID, Folder> const& AssetManager::getDirectories() const {
	return directories;
}

std::vector<FolderID> const& AssetManager::getRootDirectories() const {
	return rootDirectories;
}

void AssetManager::getAssetLoadingStatus() const {
	
}

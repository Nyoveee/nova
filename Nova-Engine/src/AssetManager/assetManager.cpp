#include <filesystem>
#include <fstream>

#include "AssetManager.h"
#include "Libraries/FileWatch.hpp"

namespace {
	// we usually don't want to increment ids
	// but in this case we want so we are explicit about it.
	void incrementFolderId(FolderID& folderId) {
		folderId = std::size_t{ folderId } + 1;
	}
}

AssetManager::AssetManager() {
	// ========= STUB CODE ==========
	addAsset<Texture>(0, "Assets/Texture/PICHU.png");
	addAsset<Model>(1, "Assets/Model/FarmTable_Textured.fbx");
	addAsset<Model>(2, "Assets/Model/box.fbx");
	addAsset<Texture>(3, "Assets/Texture/Table_frame_mtl_Base_color.png");
	addAsset<Texture>(4, "Assets/Texture/Table_top_mtl_Base_color.png");
	// ==============================

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
		std::cerr << "Filesystem error: " << ex.what() << std::endl;
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

	else if (fileExtension == ".png") {
		recordAssetFile<Texture>(path);
	}

	else {
		std::cout << "Unsupported file type of: " << path << "has been found.\n";
	}
}

AssetID AssetManager::generateAssetID(std::filesystem::path const& path) {
	(void) path;

	// STUB CODE!!
	static AssetID id{ 10 };
	id = std::size_t{ id } + 1;
	return id;
}

AssetManager::AssetMetaInfo AssetManager::parseMetaDataFile(std::filesystem::path const& path) {
	// Attempt to read corresponding metafile.
	std::string metaDataFilename = path.string() + ".nova_meta";

	std::ifstream metaDataFile{ metaDataFilename };

	if (!metaDataFile) {
		return createMetaDataFile(path);
	}

	std::string line;
	AssetID assetId;

	// reads the 1st line.
	std::getline(metaDataFile, line);

	try {
		assetId = std::stoull(line);
	}
	catch (std::exception const& exception) {
		std::cerr << "Failure when trying to parse asset id. " << exception.what() << "\n";
		return createMetaDataFile(path);
	}

	// reads the 2nd line.
	std::getline(metaDataFile, line);

	std::cout << "Successfully parsed metadata file for " << path << '\n';
	return { assetId, line };
}

AssetManager::AssetMetaInfo AssetManager::createMetaDataFile(std::filesystem::path const& path) {
	AssetID assetId = generateAssetID(path);
	std::string name = path.filename().string();

	std::string metaDataFilename = path.string() + ".nova_meta";
	std::ofstream metaDataFile{ metaDataFilename };

	if (!metaDataFile) {
		std::cerr << "Error creating metadata file: " << metaDataFilename << "!\n";
		return { assetId, name };
	}

	// write to file
	metaDataFile << static_cast<std::size_t>(assetId) << "\n" << name;
	std::cout << "Successfully created metadata file for " << path << '\n';
	return { assetId, name };
}

std::unordered_map<FolderID, Folder> const& AssetManager::getDirectories() const {
	return directories;
}

std::vector<FolderID> const& AssetManager::getRootDirectories() const {
	return rootDirectories;
}
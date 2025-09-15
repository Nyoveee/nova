#pragma once

#include <optional>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "asset.h"

class Descriptor {
public:
	// ==== Parse specific a specific asset type. ====
	// These functions will invoke the general functions below first which parses generic metadata info first
	// before performing additional parsing based on asset type.

	// retrieve an appropriate descriptor filepath based on a given intermediary asset path.
	template <ValidAsset T>
	static DescriptorFilePath getDescriptorFilename(AssetFilePath const& assetPath);

	// retrieve an appropriate resource filepath based on a given descriptor path.
	template <ValidAsset T>
	static ResourceFilePath getResourceFilename(DescriptorFilePath const& descriptorPath);

	// reads a given descriptor file.
	template <ValidAsset T>
	static std::optional<AssetInfo<T>> parseDescriptorFile(DescriptorFilePath const& descriptorFilepath);

	// creates an appropriate descriptor file based on intermediary asset.
	template <ValidAsset T>
	static AssetInfo<T> createDescriptorFile(AssetFilePath const& path);

	static ResourceID generateResourceID();

public:
	FRAMEWORK_DLL_API static const std::filesystem::path assetDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path resourceDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path descriptorDirectory;

	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subDescriptorDirectories;
	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subResourceDirectories;

private:
	// === Parse generic metadata info. These functions are invoked by the functions above first. ====
	FRAMEWORK_DLL_API static std::optional<BasicAssetInfo> parseDescriptorFile(std::ifstream& descriptorFile);
	FRAMEWORK_DLL_API static BasicAssetInfo createDescriptorFile(AssetFilePath const& path, std::ofstream& metaDataFile);
};

#include "descriptor.ipp"
#pragma once

#include <optional>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "resource.h"

// Contains shared utility functions for AssetManager, ResourceManager, Asset Compilers and Resource Loaders.
class AssetIO {
public:
	// ==== Parse specific a specific asset type. ====
	// These functions will invoke the general functions below first which parses generic metadata info first
	// before performing additional parsing based on asset type.

	// retrieve an appropriate descriptor filepath based on a given intermediary asset path.
	template <ValidResource T>
	static DescriptorFilePath getDescriptorFilename(ResourceID id);

	// retrieve an appropriate resource filepath based on a given descriptor path.
	template <ValidResource T>
	static ResourceFilePath getResourceFilename(ResourceID id);

	// reads a given descriptor file.
	template <ValidResource T>
	static std::optional<AssetInfo<T>> parseDescriptorFile(DescriptorFilePath const& descriptorFilepath);

	// creates an appropriate descriptor file based on intermediary asset.
	template <ValidResource T>
	static AssetInfo<T> createDescriptorFile(AssetFilePath const& path);

	FRAMEWORK_DLL_API static ResourceID generateResourceID();

public:
	FRAMEWORK_DLL_API static const std::filesystem::path assetDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path resourceDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path descriptorDirectory;

	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subDescriptorDirectories;
	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subResourceDirectories;

private:
	// === Parse generic metadata info. These functions are invoked by the functions above first. ====
	FRAMEWORK_DLL_API static std::optional<BasicAssetInfo> parseDescriptorFile(std::ifstream& descriptorFile);
	FRAMEWORK_DLL_API static BasicAssetInfo createDescriptorFile(ResourceID id, AssetFilePath const& path, std::ofstream& metaDataFile);
};

#include "assetIO.ipp"
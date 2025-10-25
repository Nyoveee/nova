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

	// retrieve an appropriate descriptor filepath based on resource id.
	template <ValidResource T>
	static DescriptorFilePath getDescriptorFilename(ResourceID id);

	// retrieve an appropriate resource filepath based on resource id.
	template <ValidResource T>
	static ResourceFilePath getResourceFilename(ResourceID id);

	// retrieve an appropriate asset cache filepath based on resource id.
	template <ValidResource T>
	static AssetCacheFilePath getAssetCacheFilename(ResourceID id);

	// reads a given descriptor file. (by default in respect to asset directory)
	template <ValidResource T>
	static std::optional<AssetInfo<T>> parseDescriptorFile(DescriptorFilePath const& descriptorFilepath, std::filesystem::path const& rootDirectory = assetDirectory);

	// creates an appropriate descriptor file based on intermediary asset.
	template <ValidResource T>
	static AssetInfo<T> createDescriptorFile(AssetFilePath const& path);

	// creates an appropriate descriptor file based on system asset.
	template <ValidResource T>
	static AssetInfo<T> createSystemDescriptorFile(std::filesystem::path const& systemPath, DescriptorFilePath const& descriptorFilePath);

	// create an appropriate descriptor file.
	template <ValidResource T>
	static AssetInfo<T> createDescriptorFile(ResourceID id, std::filesystem::path const& assetPath, std::ostream& descriptorFile, std::filesystem::path const& rootDirectory);

	FRAMEWORK_DLL_API static ResourceID generateResourceID();

public:
	FRAMEWORK_DLL_API static const std::filesystem::path assetDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path resourceDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path descriptorDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path assetCacheDirectory;
	FRAMEWORK_DLL_API static const std::filesystem::path systemResourceDirectory;

	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subDescriptorDirectories;
	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subResourceDirectories;
	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subAssetCacheDirectories;
	FRAMEWORK_DLL_API static const std::unordered_map<ResourceTypeID, std::filesystem::path> subSystemResourceDirectories;

private:
	// === Parse generic metadata info. These functions are invoked by the functions above first. ====
	FRAMEWORK_DLL_API static std::optional<BasicAssetInfo> parseDescriptorFile(std::ifstream& descriptorFile, std::filesystem::path const& rootDirectory);
	FRAMEWORK_DLL_API static BasicAssetInfo createDescriptorFile(ResourceID id, std::filesystem::path const& assetPath, std::ostream& descriptorFile, std::filesystem::path const& rootDirectory);
};

#include "assetIO.ipp"
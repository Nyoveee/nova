#include "Logger.h"
#include "assetIO.h"

#include <filesystem>
#include <sstream>

// ========================================================
// RESOURCE FILE FORMAT
// 1st 8 bytes -> ResourceID
// rest of the bytes -> custom file format.
// ========================================================
template <ValidResource T>
int Compiler::compileAsset(DescriptorFilePath const& descriptorFilepath) {
	// Retrieve descriptor info.
	std::optional<AssetInfo<T>> optAssetInfo = AssetIO::parseDescriptorFile<T>(descriptorFilepath);

	if (!optAssetInfo) {
		Logger::error("Failed to parse descriptor file: {}. Compilation failed.", descriptorFilepath.string);
		return -1;
	}

	AssetInfo<T> assetInfo = optAssetInfo.value();
	ResourceFilePath resourceFilePath = AssetIO::getResourceFilename<T>(assetInfo.id);

	if constexpr (std::same_as<T, Texture>) {
		return compileTexture(resourceFilePath, assetInfo.filepath, assetInfo.compression);
	}
	else if constexpr (std::same_as<T, Model>) {
		return compileModel(resourceFilePath, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, CubeMap>) {
		return compileTexture(resourceFilePath, assetInfo.filepath, AssetInfo<Texture>::Compression::BC6H);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		return compileScriptAsset(resourceFilePath, assetInfo.name);
	}
	else if constexpr (std::same_as<T, CustomShader>) {
		return compileShaderAsset(resourceFilePath, assetInfo.filepath, assetInfo.pipeline);
	}
	else {
		return defaultCompile(resourceFilePath, assetInfo.filepath);
	}
}
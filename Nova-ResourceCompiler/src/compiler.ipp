#include "texture.h"
#include "audio.h"
#include "cubemap.h"
#include "model.h"
#include "scriptAsset.h"
#include "Logger.h"
#include "assetIO.h"

#include <filesystem>
#include <sstream>

// ========================================================
// RESOURCE FILE FORMAT
// 1st 8 bytes -> ResourceID
// rest of the bytes -> custom file format.
// ========================================================
template <ValidAsset T>
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
		return compileTexture(resourceFilePath, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, Model>) {
		return compileModel(resourceFilePath, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, CubeMap>) {
		return compileCubeMap(resourceFilePath, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		return compileScriptAsset(resourceFilePath, assetInfo.filepath);
	}
	else {
		return defaultCompile(resourceFilePath, assetInfo.filepath);
	}
}
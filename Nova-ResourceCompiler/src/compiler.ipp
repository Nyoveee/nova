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
void Compiler::compileAsset(std::filesystem::path const& descriptorFilepath) {
	// Retrieve descriptor info.
	std::optional<AssetInfo<T>> optAssetInfo = AssetIO::parseDescriptorFile<T>(descriptorFilepath);

	if (!optAssetInfo) {
		Logger::error("Failed to parse descriptor file: {}. Compilation failed.", descriptorFilepath.string());
		return;
	}

	AssetInfo<T> assetInfo = optAssetInfo.value();

	if constexpr (std::same_as<T, Texture>) {
		compileTexture(AssetIO::getResourceFilename<T>(assetInfo.id), assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, Model>) {
		compileModel(AssetIO::getResourceFilename<T>(assetInfo.id), assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, CubeMap>) {
		compileCubeMap(AssetIO::getResourceFilename<T>(assetInfo.id), assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, Audio>) {
		compileAudio(AssetIO::getResourceFilename<T>(assetInfo.id), assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		compileScriptAsset(AssetIO::getResourceFilename<T>(assetInfo.id), assetInfo.filepath);
	}
	else {
		[] <bool flag = true>() {
			static_assert(flag, "Compiling a not supported asset type.");
		}();
	}
}
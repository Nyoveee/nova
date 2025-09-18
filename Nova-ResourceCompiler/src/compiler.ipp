#include "texture.h"
#include "audio.h"
#include "cubemap.h"
#include "model.h"
#include "scriptAsset.h"
#include "Logger.h"
#include "descriptor.h"

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
	std::optional<AssetInfo<T>> optAssetInfo = Descriptor::parseDescriptorFile<T>(descriptorFilepath);

	if (!optAssetInfo) {
		Logger::error("Failed to parse descriptor file: {}. Compilation failed.", descriptorFilepath.string());
		return;
	}

	AssetInfo<T> assetInfo = optAssetInfo.value();

	std::ofstream resourceFile { Descriptor::getResourceFilename<T>(assetInfo.id), std::ios::binary };

	if (!resourceFile) {
		Logger::error("Failed to create resource file: {}. Compilation failed.", Descriptor::getResourceFilename<T>(assetInfo.id).string);
		return;
	}

	// 1st 8 bytes for resource id.
	std::size_t resourceId{ assetInfo.id };
	Logger::info("Generated resource id: {}", resourceId);

	resourceFile.write(reinterpret_cast<const char*>(&resourceId), sizeof(resourceId));

	if constexpr (std::same_as<T, Texture>) {
		compileTexture(resourceFile, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, Model>) {
		compileModel(resourceFile, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, CubeMap>) {
		compileCubeMap(resourceFile, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, Audio>) {
		compileAudio(resourceFile, assetInfo.filepath);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		compileScriptAsset(resourceFile, assetInfo.filepath);
	}
	else {
		[] <bool flag = true>() {
			static_assert(flag, "Compiling a not supported asset type.");
		}();
	}
}
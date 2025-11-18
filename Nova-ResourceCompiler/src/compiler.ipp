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
int Compiler::compileAsset(AssetInfo<T> const& assetInfo, ResourceFilePath const& resourceFilePath) {
	if constexpr (std::same_as<T, Texture>) {
		return compileTexture(resourceFilePath, assetInfo.filepath, assetInfo.compression);
	}
	else if constexpr (std::same_as<T, Model>) {
		return compileModel(resourceFilePath, assetInfo.filepath, assetInfo.scale);
	}
	else if constexpr (std::same_as<T, CubeMap>) {
		return compileTexture(resourceFilePath, assetInfo.filepath, AssetInfo<Texture>::Compression::BC6H);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		return compileScriptAsset(resourceFilePath, assetInfo.filepath, assetInfo.name);
	}
	else if constexpr (std::same_as<T, CustomShader>) {
		return compileShaderAsset(resourceFilePath, assetInfo.filepath, assetInfo.pipeline);
	}
	else if constexpr (std::same_as<T, Font>) {
		return compileFont(resourceFilePath, assetInfo.filepath, assetInfo.fontSize);
	}
	else {
		return defaultCompile(resourceFilePath, assetInfo.filepath);
	}
}
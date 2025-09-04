#pragma once
/*
#include "asset.h"

using GLuint = unsigned int;

class Audio : public Asset {
public:
	DLL_API Audio(std::string filepath);
	DLL_API ~Audio();

	DLL_API Audio(Audio const& other) = delete;
	DLL_API Audio(Audio&& other) = default;
	DLL_API Audio& operator=(Audio const& other) = delete;
	DLL_API Audio& operator=(Audio&& other) noexcept;

public:
	// Leave blank as loading and unloading is done in audioSystem
	DLL_API void load(AssetManager& assetManager) final;
	DLL_API void unload() final;

private:
	int numChannels;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!

// template <>
// struct AssetInfo<Texture> : public BasicAssetInfo {
// 	bool isFlipped = false;
// };

// Explicitly define how to construct your type given asset info
// inline keyword for multiple definitions of the same function.
// template <>
// inline Texture createAsset(AssetInfo<Texture> const& assetInfo) {
//		return Texture{ assetInfo.filepath, assetInfo.isFlipped };
// }
*/
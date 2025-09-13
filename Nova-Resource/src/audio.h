#pragma once

#include "asset.h"

class Audio : public Asset {
public:
	DLL_API Audio(std::string filepath, bool is3D = false);
	DLL_API ~Audio();

	DLL_API Audio(Audio const& other) = delete;
	DLL_API Audio(Audio&& other) = default;
	DLL_API Audio& operator=(Audio const& other) = delete;
	DLL_API Audio& operator=(Audio&& other) = default;

public:
	// Leave blank as loading and unloading is done in audioSystem
	DLL_API void load() final;
	DLL_API void unload() final;
	DLL_API std::string getClassName() const;

public:
	DLL_API bool isAudio3D() const;

private:
	bool is3D;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!
template <>
struct AssetInfo<Audio> : public BasicAssetInfo {
	bool is3D;
};

// Explicitly define how to construct your type given asset info
// inline keyword for multiple definitions of the same function.
template <>
inline Audio createAsset(AssetInfo<Audio> const& assetInfo) {
	return Audio{ assetInfo.filepath, assetInfo.is3D };
}
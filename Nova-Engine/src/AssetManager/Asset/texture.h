#pragma once

#include "asset.h"

using GLuint = unsigned int;

class Texture : public Asset {
public:
	DLL_API Texture(std::string filepath, bool toFlip);
	DLL_API ~Texture();

	DLL_API Texture(Texture const& other) = delete;
	DLL_API Texture(Texture&& other) noexcept;
	DLL_API Texture& operator=(Texture const& other) = delete;
	DLL_API Texture& operator=(Texture&& other) noexcept;

public:
	DLL_API void load() final;
	DLL_API void unload() final;

public:
	DLL_API GLuint getTextureId() const;
	DLL_API bool isFlipped() const;

private:
	GLuint textureId;
	int width;
	int height;
	int numChannels;

	bool toFlip;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!
template <>
struct AssetInfo<Texture> : public BasicAssetInfo {
	bool isFlipped = false;
};

// Explicitly define how to construct your type given asset info
// inline keyword for multiple definitions of the same function.
template <>
inline Texture createAsset(AssetInfo<Texture> const& assetInfo) { 
	return Texture{ assetInfo.filepath, assetInfo.isFlipped }; 
}
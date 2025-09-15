#pragma once

#include "asset.h"

using GLuint = unsigned int; 

class Texture : public Asset {
public:
	FRAMEWORK_DLL_API Texture(ResourceFilePath filepath);
	FRAMEWORK_DLL_API ~Texture();

	FRAMEWORK_DLL_API Texture(Texture const& other) = delete;
	FRAMEWORK_DLL_API Texture(Texture&& other) noexcept;
	FRAMEWORK_DLL_API Texture& operator=(Texture const& other) = delete;
	FRAMEWORK_DLL_API Texture& operator=(Texture&& other) noexcept;

public:
	FRAMEWORK_DLL_API bool load() final;
	FRAMEWORK_DLL_API void unload() final;

public:
	FRAMEWORK_DLL_API GLuint getTextureId() const;
	FRAMEWORK_DLL_API bool isFlipped() const;

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
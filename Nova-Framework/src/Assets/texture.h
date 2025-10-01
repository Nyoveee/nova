#pragma once

#include "resource.h"
#include <gli/gli.hpp>

using GLuint = unsigned int; 

class Texture : public Resource {
public:
	FRAMEWORK_DLL_API Texture(ResourceID id, ResourceFilePath resourceFilePath, gli::texture const& texture, gli::gl::format const& format);
	FRAMEWORK_DLL_API ~Texture();

	FRAMEWORK_DLL_API Texture(Texture const& other) = delete;
	FRAMEWORK_DLL_API Texture(Texture&& other) noexcept;
	FRAMEWORK_DLL_API Texture& operator=(Texture const& other) = delete;
	FRAMEWORK_DLL_API Texture& operator=(Texture&& other) noexcept;

public:
	FRAMEWORK_DLL_API GLuint getTextureId() const;
	FRAMEWORK_DLL_API bool isFlipped() const;

private:
	GLuint textureId;
	int width;
	int height;
};

template <>
struct AssetInfo<Texture> : public BasicAssetInfo {
	enum class Compression {
		Uncompressed_Linear,
		Uncompressed_SRGB,
		BC1_SRGB,				// for most textures, with no alpha.
		BC5,					// for normal maps.. (only store x and y)..			
		BC6H,					// for HDR maps, linear only.
		BC7_SRGB				// for high quality texture.
	} compression;
};
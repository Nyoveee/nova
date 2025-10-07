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
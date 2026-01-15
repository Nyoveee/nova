#pragma once

#include "resource.h"
#include <gli/gli.hpp>

using GLuint = unsigned int;

class CubeMap : public Resource {
public:
	// this constructor constructs a cube map resource, which the resource manager will be in charge of.
	FRAMEWORK_DLL_API CubeMap(ResourceID id, ResourceFilePath resourceFilePath, gli::texture const& texture, gli::gl::format const& format);
	
	// this constructor creates a cube map that is not part of the resource manager, but rather just responsible for owning the data of the cubemap.
	// we initialise the cube map to null. this is used to be a target for baking irradiance maps.
	FRAMEWORK_DLL_API CubeMap(int width, int height, int mipmapLevel = 1);

	FRAMEWORK_DLL_API ~CubeMap();

	FRAMEWORK_DLL_API CubeMap(CubeMap const& other) = delete;
	FRAMEWORK_DLL_API CubeMap(CubeMap&& other) noexcept;
	FRAMEWORK_DLL_API CubeMap& operator=(CubeMap const& other) = delete;
	FRAMEWORK_DLL_API CubeMap& operator=(CubeMap&& other) noexcept;

public:
	FRAMEWORK_DLL_API GLuint getTextureId() const;
	FRAMEWORK_DLL_API int getWidth() const;
	FRAMEWORK_DLL_API int getHeight() const;
	FRAMEWORK_DLL_API int getMipmap() const;

private:
	GLuint textureId;

	int width;
	int height;
	int mipmapLevels;
};
#pragma once

#include "resource.h"
#include <gli/gli.hpp>

using GLuint = unsigned int;

class EquirectangularMap : public Resource {
public:
	FRAMEWORK_DLL_API EquirectangularMap(ResourceID id, ResourceFilePath resourceFilePath, gli::texture const& texture, gli::gl::format const& format);
	FRAMEWORK_DLL_API ~EquirectangularMap();

	FRAMEWORK_DLL_API EquirectangularMap(EquirectangularMap  const& other) = delete;
	FRAMEWORK_DLL_API EquirectangularMap(EquirectangularMap&& other) noexcept;
	FRAMEWORK_DLL_API EquirectangularMap& operator=(EquirectangularMap  const& other) = delete;
	FRAMEWORK_DLL_API EquirectangularMap& operator=(EquirectangularMap&& other) noexcept;

public:
	FRAMEWORK_DLL_API GLuint getTextureId() const;

public:
	GLuint textureId;
	int width;
	int height;
};
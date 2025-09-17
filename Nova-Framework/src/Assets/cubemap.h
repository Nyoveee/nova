#pragma once

#include "asset.h"

using GLuint = unsigned int;

class CubeMap : public Asset {
public:
	FRAMEWORK_DLL_API CubeMap(ResourceID id);
	FRAMEWORK_DLL_API ~CubeMap();

	FRAMEWORK_DLL_API CubeMap(CubeMap const& other) = delete;
	FRAMEWORK_DLL_API CubeMap(CubeMap&& other) noexcept;
	FRAMEWORK_DLL_API CubeMap& operator=(CubeMap const& other) = delete;
	FRAMEWORK_DLL_API CubeMap& operator=(CubeMap&& other) noexcept;

public:
	FRAMEWORK_DLL_API GLuint getTextureId() const;

public:
	GLuint textureId;
	int width;
	int height;
	int numChannels;
};
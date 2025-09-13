#pragma once

#include "asset.h"

using GLuint = unsigned int;

class CubeMap : public Asset {
public:
	DLL_API CubeMap(std::string filepath);
	DLL_API ~CubeMap();

	DLL_API CubeMap(CubeMap const& other) = delete;
	DLL_API CubeMap(CubeMap&& other) noexcept;
	DLL_API CubeMap& operator=(CubeMap const& other) = delete;
	DLL_API CubeMap& operator=(CubeMap&& other) noexcept;

public:
	DLL_API void load() final;
	DLL_API void unload() final;

public:
	DLL_API GLuint getTextureId() const;

public:
	GLuint textureId;
	int width;
	int height;
	int numChannels;
};
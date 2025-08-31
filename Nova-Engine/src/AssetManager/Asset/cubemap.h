#pragma once

#include "asset.h"

using GLuint = unsigned int;

class CubeMap : public Asset {
public:
	CubeMap(std::string filepath);
	~CubeMap();

	CubeMap(CubeMap const& other)				= delete;
	CubeMap(CubeMap&& other) noexcept;
	CubeMap& operator=(CubeMap const& other)	= delete;
	CubeMap& operator=(CubeMap&& other) noexcept;

public:
	void load(AssetManager& assetManager) final;
	void unload() final;

public:
	GLuint getTextureId() const;

public:
	GLuint textureId;
	int width;
	int height;
	int numChannels;
};
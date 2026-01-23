#pragma once

using GLuint = unsigned int;

class CubeMapArray {
public:
	CubeMapArray(int width, int height, int size, int mipmapLevel = 1);
	~CubeMapArray();

	// i cba to code move semantics anymore.
	CubeMapArray(CubeMapArray const& other)				= delete;
	CubeMapArray(CubeMapArray&& other)					= delete;
	CubeMapArray& operator=(CubeMapArray const& other)	= delete;
	CubeMapArray& operator=(CubeMapArray&& other)		= delete;

public:
	GLuint getTextureId() const;

	int getWidth() const;
	int getHeight() const;
	int getSize() const;
	int getMipmap() const;

private:
	GLuint textureId;

	int width;
	int height;
	int size;
	int mipmapLevels;
};
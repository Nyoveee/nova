#pragma once

using GLuint = unsigned int;
using GLenum = unsigned int;

class Texture2DArray {
public:
	Texture2DArray(int width, int height, GLenum internalFormat, int layers, int mipmapLevels = 1);
	~Texture2DArray();

	Texture2DArray(Texture2DArray const& other) = delete;
	Texture2DArray(Texture2DArray&& other) noexcept;
	Texture2DArray& operator=(Texture2DArray const& other) = delete;
	Texture2DArray& operator=(Texture2DArray&& other) noexcept;

public:
	GLuint getTextureId() const;
	int getWidth() const;
	int getHeight() const;

private:
	GLuint textureId;
	int width;
	int height;
	int layers;
	int mipmapLevels;
};
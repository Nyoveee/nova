#include "texture2dArray.h"

#include <glad/glad.h>
#include <iostream>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

Texture2DArray::Texture2DArray(int width, int height, GLenum internalFormat, int layers, int mipmapLevels) :
	width			{ width },
	height			{ height },
	textureId		{ TEXTURE_NOT_LOADED },
	layers			{ layers },
	mipmapLevels	{ mipmapLevels }
{
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);
	glTextureStorage3D(textureId, mipmapLevels, internalFormat, width, height, layers);

	glTextureParameteri(textureId, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(textureId, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mipmapLevels - 1));

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2DArray::~Texture2DArray() {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}
}

Texture2DArray::Texture2DArray(Texture2DArray&& other) noexcept :
	width			{ other.width },
	height			{ other.height },
	layers			{ other.layers },
	mipmapLevels	{ other.mipmapLevels },
	textureId		{ other.textureId }
{
	other.textureId = TEXTURE_NOT_LOADED;
}

Texture2DArray& Texture2DArray::operator=(Texture2DArray&& other) noexcept {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}

	width = other.width;
	height = other.height;
	layers = other.layers;
	mipmapLevels = other.mipmapLevels;
	textureId = other.textureId;

	other.textureId = TEXTURE_NOT_LOADED;

	return *this;
}

GLuint Texture2DArray::getTextureId() const {
	return textureId;
}

int Texture2DArray::getWidth() const {
	return width;
}

int Texture2DArray::getHeight() const {
	return height;
}

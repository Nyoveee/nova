#include "cubemapArray.h"

#include <glad/glad.h>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

CubeMapArray::CubeMapArray(int width, int height, int size, int mipmapLevel) :
	textureId		{ TEXTURE_NOT_LOADED },
	width			{ width },
	height			{ height },
	size			{ size },
	mipmapLevels	{ mipmapLevel }
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &textureId);
	glTextureStorage3D(textureId, mipmapLevel, GL_RGBA16F, width, height, size * 6);

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

CubeMapArray::~CubeMapArray() {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}
}

#if 0
CubeMap::CubeMap(CubeMap&& other) noexcept :
	Resource{ std::move(other) },
	width{ other.width },
	height{ other.height },
	textureId{ other.textureId },
	mipmapLevels{ other.mipmapLevels }
{
	other.textureId = TEXTURE_NOT_LOADED;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept {
	Resource::operator=(std::move(other));

	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}

	width = other.width;
	height = other.height;
	textureId = other.textureId;
	mipmapLevels = other.mipmapLevels;

	other.textureId = TEXTURE_NOT_LOADED;

	return *this;
}
#endif

GLuint CubeMapArray::getTextureId() const {
	return textureId;
}

int CubeMapArray::getWidth() const {
	return width;
}

int CubeMapArray::getHeight() const {
	return height;
}

int CubeMapArray::getSize() const {
	return size;
}

int CubeMapArray::getMipmap() const {
	return mipmapLevels;
}

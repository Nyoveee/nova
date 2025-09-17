#include "texture.h"

#include <glad/glad.h>
#include <iostream>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

Texture::Texture(ResourceID id) :
	Asset			 { id },
	width			 {},
	height			 {},
	numChannels		 {},
	textureId		 { TEXTURE_NOT_LOADED }
{
#if 0
	if (isLoaded()) {
		Logger::error("Attempting to load texture when there's already something loaded!");
		return;
	}

	//int width, height, numChannels;
	unsigned char* data = stbi_load(getFilePath().c_str(), &width, &height, &numChannels, 0);

	if (!data) {
		Logger::error("Failed to load texture! Filepath provided: {}", getFilePath());
		loadStatus = Asset::LoadStatus::LoadingFailed;
		return;
	}

	//this->width = width;
	//this->height = height;
	//this->numChannels = numChannels;

	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

	// black and white binary.
	GLint internalFormat;
	GLint format;

	if (numChannels == 1) {
		internalFormat = GL_R8;
		format = GL_RED;
	}
	else if (numChannels == 3) {
		internalFormat = GL_RGB16;
		format = GL_RGB;
	}
	else if (numChannels == 4) {
		internalFormat = GL_RGBA16;
		format = GL_RGBA;
	}
	else {
		Logger::error("Weird number of channels? {} has {} channels? Texture not created.", getFilePath(), numChannels);
		return;
	}

	glTextureStorage2D(textureId, 1, internalFormat, width, height);
	glTextureSubImage2D(textureId, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateTextureMipmap(textureId);
	stbi_image_free(data);

	loadStatus = Asset::LoadStatus::Loaded;
	//TracyAlloc(this, sizeof(*this));
#endif
}

Texture::~Texture() {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}
}

Texture::Texture(Texture&& other) noexcept :
	Asset			 { std::move(other) },
	width			 { other.width },
	height			 { other.height },
	numChannels		 { other.numChannels },
	textureId		 { other.textureId },
	toFlip			 { other.toFlip }
{
	other.textureId = TEXTURE_NOT_LOADED;
}

Texture& Texture::operator=(Texture&& other) noexcept {
	Asset::operator=(std::move(other));

	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}

	width			 = other.width;
	height			 = other.height;
	numChannels		 = other.numChannels;
	textureId		 = other.textureId;
	toFlip			 = other.toFlip;

	other.textureId = TEXTURE_NOT_LOADED;

	return *this;
}

GLuint Texture::getTextureId() const {
	return textureId;
}

bool Texture::isFlipped() const {
	return toFlip;
}

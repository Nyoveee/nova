#include <spdlog/spdlog.h>

#include "texture.h"
#include "Libraries/stb_image.hpp"

#include <glad/glad.h>
#include <iostream>

Texture::Texture(std::string filepath, bool toFlip) :
	Asset			 { filepath },
	width			 {},
	height			 {},
	numChannels		 {},
	textureId		 {},
	toFlip			 { toFlip }
{}

Texture::~Texture() {
	if (!hasLoaded) {
		return;
	}

	unload();
}

Texture::Texture(Texture&& other) noexcept :
	Asset			 { std::move(other) },
	width			 { other.width },
	height			 { other.height },
	numChannels		 { other.numChannels },
	textureId		 { other.textureId },
	toFlip			 { other.toFlip }
{
	other.textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
	Asset::operator=(std::move(other));

	if(hasLoaded) unload();

	width			 = other.width;
	height			 = other.height;
	numChannels		 = other.numChannels;
	textureId		 = other.textureId;
	toFlip			 = other.toFlip;

	other.textureId = 0;

	return *this;
}

void Texture::load() {
	if (hasLoaded) {
		spdlog::error("Attempting to load texture when there's already something loaded!");
		return;
	}

	if (toFlip) {
		stbi_set_flip_vertically_on_load(true);
	}
	else {
		stbi_set_flip_vertically_on_load(false);
	}

	unsigned char* data = stbi_load(getFilePath().c_str(), &width, &height, &numChannels, 0);

	if (!data) {
		spdlog::error("Failed to load texture! Filepath provided: {}", getFilePath());
		return;
	}

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
		spdlog::error("Weird number of channels? {} has {} channels? Texture not created.", getFilePath(), numChannels);
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

	hasLoaded = true;
}

void Texture::unload() {
	glDeleteTextures(1, &textureId);
	hasLoaded = false;
}

GLuint Texture::getTextureId() const {
	return textureId;
}

DLL_API bool Texture::isFlipped() const {
	return toFlip;
}

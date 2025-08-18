#include "texture.h"
#include "Libraries/stb_image.hpp"

#include <glad/glad.h>
#include <iostream>

Texture::Texture(std::string filepath) :
	Asset			 { filepath },
	width			 {},
	height			 {},
	numChannels		 {},
	textureId		 {}
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
	textureId		 { other.textureId }
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

	other.textureId = 0;

	return *this;
}

void Texture::load() {
	if (hasLoaded) {
		std::cerr << "Attempting to load texture when there's already something loaded!\n";
		return;
	}

	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(getFilePath().c_str(), &width, &height, &numChannels, 0);

	if (!data) {
		std::cerr << "Failed to load texture! Filepath provided: " + getFilePath() + "\n";
		return;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

	GLint internalFormat = numChannels == 4 ? GL_RGBA16 : GL_RGB16;
	GLint format = numChannels == 4 ? GL_RGBA : GL_RGB;

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

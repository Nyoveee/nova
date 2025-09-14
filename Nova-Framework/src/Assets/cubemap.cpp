#include "cubemap.h"
//#include "tinyexr.h"

#include <glad/glad.h>
#include "Logger.h"

CubeMap::CubeMap(std::string filepath) :
	Asset			 { filepath },
	width			 {},
	height			 {},
	numChannels		 {},
	textureId		 {}
{}

CubeMap::~CubeMap() {
	if (!isLoaded()) {
		return;
	}

	unload();
}

CubeMap::CubeMap(CubeMap&& other) noexcept :
	Asset			 { std::move(other) },
	width			 { other.width },
	height			 { other.height },
	numChannels		 { other.numChannels },
	textureId		 { other.textureId }
{
	other.textureId = 0;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept {
	Asset::operator=(std::move(other));

	if(isLoaded()) unload();

	width			 = other.width;
	height			 = other.height;
	numChannels		 = other.numChannels;
	textureId		 = other.textureId;

	other.textureId = 0;

	return *this;
}

void CubeMap::load() {
#if 0
	if (isLoaded()) {
		Logger::error("Attempting to load texture when there's already something loaded!");
		return;
	}
	
	float* data; // width * height * RGBA
	//int width;
	//int height;
	const char* err = nullptr; // or nullptr in C++11

	int status = LoadEXR(&data, &width, &height, getFilePath().c_str(), &err);

	if (status != TINYEXR_SUCCESS) {
		if (err) {
			Logger::error("ERR : {}", err);
			FreeEXRErrorMessage(err); // release memory of error message.
		}

		Asset::loadStatus = Asset::LoadStatus::LoadingFailed;
		return;
	}

	// im assuming GL_RGBA16F format.
	// @TODO: actually check the header file of OpenEXR.
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

	glTextureStorage2D(textureId, 1, GL_RGBA16F, width, height);
	glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateTextureMipmap(textureId);
	free(data); // release memory of image data

	this->width = width;
	this->height = height;

	loadStatus = Asset::LoadStatus::Loaded;
	//TracyAlloc(this, sizeof(*this));
#endif
	loadStatus = Asset::LoadStatus::LoadingFailed;
}

void CubeMap::unload() {
	glDeleteTextures(1, &textureId);
	loadStatus = Asset::LoadStatus::NotLoaded;
	
	//TracyFree(this);
}

GLuint CubeMap::getTextureId() const {
	return textureId;
}
#include "cubemap.h"

#include <glad/glad.h>
#include <iostream>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

CubeMap::CubeMap(ResourceID id, ResourceFilePath resourceFilePath, gli::texture const& texture, gli::gl::format const& format) :
	Resource	{ id, std::move(resourceFilePath) },
	width		{ texture.extent().x },
	height		{ texture.extent().y },
	textureId	{ TEXTURE_NOT_LOADED },
	mipmapLevels{ static_cast<int>(texture.levels()) }
{
	gli::gl GL(gli::gl::PROFILE_GL33);
	GLuint target = GL.translate(texture.target());

	if (target != GL_TEXTURE_CUBE_MAP) {
		Logger::error("DDS file is not a cube map!");
		return;
	}

	glCreateTextures(target, 1, &textureId);
	glTextureStorage2D(textureId, static_cast<GLint>(texture.levels()), format.Internal, width, height);

	for (std::size_t layer = 0; layer < texture.layers(); ++layer) {
		for (std::size_t face = 0; face < texture.faces(); ++face) {
			for (std::size_t level = 0; level < texture.levels(); ++level) {	
				auto extent = texture.extent(level);

				if (gli::is_compressed(texture.format())) {
					glCompressedTextureSubImage3D(
						textureId,
						static_cast<GLint>(level),
						0, 0, static_cast<GLint>(face),	// offsets, we offset based on face
						extent.x,						// width
						extent.y,						// height
						1,								// depth, we upload one layer / depth at a time.
						format.Internal,
						static_cast<GLsizei>(texture.size(level)),
						texture.data(layer, face, level)
					);
				}
				else {
					glTextureSubImage3D(
						textureId,
						static_cast<GLint>(level),
						0, 0, static_cast<GLint>(face),	// offsets, we offset based on face
						extent.x,						// width
						extent.y,						// height
						1,								// depth, we upload one layer / depth at a time.
						format.External,
						format.Type,
						texture.data(layer, face, level)
					);
				}
			}
		}
	}

	glTextureParameteri(textureId, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(textureId, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);
}

CubeMap::CubeMap(int width, int height, int mipmapLevel) :
	Resource		{ INVALID_RESOURCE_ID, ResourceFilePath{}},
	textureId		{ TEXTURE_NOT_LOADED },
	width			{ width },
	height			{ height },
	mipmapLevels	{ mipmapLevel }
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureId);
	glTextureStorage2D(textureId, mipmapLevel, GL_RGBA16F, width, height);

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

CubeMap::~CubeMap() {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}
}

CubeMap::CubeMap(CubeMap&& other) noexcept :
	Resource		{ std::move(other) },
	width			{ other.width },
	height			{ other.height },
	textureId		{ other.textureId },
	mipmapLevels	{ other.mipmapLevels }
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

GLuint CubeMap::getTextureId() const {
	return textureId;
}

int CubeMap::getWidth() const {
	return width;
}

int CubeMap::getHeight() const {
	return height;
}

int CubeMap::getMipmap() const {
	return mipmapLevels;
}

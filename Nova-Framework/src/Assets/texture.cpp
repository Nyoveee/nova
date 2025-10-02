#include "texture.h"

#include <glad/glad.h>
#include <iostream>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

Texture::Texture(ResourceID id, ResourceFilePath resourceFilePath, gli::texture const& texture, gli::gl::format const& format) :
	Resource		 { id, std::move(resourceFilePath) },
	width			 { texture.extent().x },
	height			 { texture.extent().y },
	textureId		 { TEXTURE_NOT_LOADED }
{
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
	glTextureStorage2D(textureId, static_cast<GLint>(texture.levels()), format.Internal, width, height);

	for (std::size_t layer = 0; layer < texture.layers(); ++layer) {
		for (std::size_t face = 0; face < texture.faces(); ++face) {
			for (std::size_t level = 0; level < texture.levels(); ++level) {
				auto extent = texture.extent(level);

				if (gli::is_compressed(texture.format())) {
					glCompressedTextureSubImage2D(
						textureId,
						static_cast<GLint>(level),
						0, 0,	// offsets
						extent.x,
						extent.y,
						format.Internal, 
						static_cast<GLsizei>(texture.size(level)),
						texture.data(layer, face, level)
					);
				}
				else {
					glTextureSubImage2D(
						textureId, 
						static_cast<GLint>(level),
						0, 0,	// offsets
						extent.x,
						extent.y,
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

	glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
	glTextureParameteri(textureId, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);
}

Texture::~Texture() {
	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}
}

Texture::Texture(Texture&& other) noexcept :
	Resource		 { std::move(other) },
	width			 { other.width },
	height			 { other.height },
	textureId		 { other.textureId }
{
	other.textureId = TEXTURE_NOT_LOADED;
}

Texture& Texture::operator=(Texture&& other) noexcept {
	Resource::operator=(std::move(other));

	if (textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &textureId);
	}

	width			 = other.width;
	height			 = other.height;
	textureId		 = other.textureId;

	other.textureId = TEXTURE_NOT_LOADED;

	return *this;
}

GLuint Texture::getTextureId() const {
	return textureId;
}

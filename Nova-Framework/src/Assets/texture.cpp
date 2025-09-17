#include "texture.h"

#include <glad/glad.h>
#include <iostream>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

Texture::Texture(ResourceID id, gli::texture const& texture, gli::gl::format const& format) :
	Asset			 { id },
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

#if 0
	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(texture.format(), texture.swizzles());
	GLenum Target = GL.translate(texture.target());

	GLuint textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(Target, textureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

	glm::tvec3<GLsizei> const Extent(texture.extent());
	GLsizei const FaceTotal = static_cast<GLsizei>(texture.layers() * texture.faces());

	switch(texture.target())
	{
	case gli::TARGET_1D:
		glTexStorage1D(
			Target, static_cast<GLint>(texture.levels()), Format.Internal, Extent.x);
		break;
	case gli::TARGET_1D_ARRAY:
	case gli::TARGET_2D:
	case gli::TARGET_CUBE:
		glTexStorage2D(
			Target, static_cast<GLint>(texture.levels()), Format.Internal,
			Extent.x, texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
		break;
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_3D:
	case gli::TARGET_CUBE_ARRAY:
		glTexStorage3D(
			Target, static_cast<GLint>(texture.levels()), Format.Internal,
			Extent.x, Extent.y,
			texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
		break;
	default:
		assert(0);
		break;
	}

	for(std::size_t Layer = 0; Layer < texture.layers(); ++Layer)
	for(std::size_t Face = 0; Face < texture.faces(); ++Face)
	for(std::size_t Level = 0; Level < texture.levels(); ++Level)
	{
		GLsizei const LayerGL = static_cast<GLsizei>(Layer);
		glm::tvec3<GLsizei> Extent(texture.extent(Level));
		Target = gli::is_target_cube(texture.target())
			? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
			: Target;

		switch(texture.target())
		{
		case gli::TARGET_1D:
			if(gli::is_compressed(texture.format()))
				glCompressedTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.Internal, static_cast<GLsizei>(texture.size(Level)),
					texture.data(Layer, Face, Level));
			else
				glTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.External, Format.Type,
					texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			if(gli::is_compressed(texture.format()))
				glCompressedTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.Internal, static_cast<GLsizei>(texture.size(Level)),
					texture.data(Layer, Face, Level));
			else
				glTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.External, Format.Type,
					texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			if(gli::is_compressed(texture.format()))
				glCompressedTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.Internal, static_cast<GLsizei>(texture.size(Level)),
					texture.data(Layer, Face, Level));
			else
				glTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.External, Format.Type,
					texture.data(Layer, Face, Level));
			break;
		default: assert(0); break;
		}
	}

	textureId = textureName;
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
	textureId		 { other.textureId }
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
	textureId		 = other.textureId;

	other.textureId = TEXTURE_NOT_LOADED;

	return *this;
}

GLuint Texture::getTextureId() const {
	return textureId;
}

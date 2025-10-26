#include "font.h"

#include <glad/glad.h>
#include <iostream>


#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
};

Font::Font(ResourceID id, ResourceFilePath resourceFilePath, Data data) :
    Resource	{ id, resourceFilePath },
    atlas		{ TEXTURE_NOT_LOADED, data.atlasWidth, data.atlasHeight },
	characters	{ std::move(data.characters) },
	fontSize	{ data.fontSize }
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    // generate texture
    GLuint atlasTex = TEXTURE_NOT_LOADED;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.width, atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int xOffset = 0;

	for (auto const& sprite : data.sprites) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, 0, sprite.width, sprite.height, GL_RED, GL_UNSIGNED_BYTE, sprite.bytes.data());
		xOffset += sprite.width;
	}

    atlas.textureId = atlasTex;
}

Font::Font(Font&& other) noexcept :
	Resource	{ std::move(other) },
	atlas		{ other.atlas },
	characters	{ std::move(other.characters) },
	fontSize	{ other.fontSize }
{
	other.atlas.textureId = TEXTURE_NOT_LOADED;
}

Font& Font::operator=(Font&& other) noexcept {
	Resource::operator=(std::move(other));

	if (atlas.textureId != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &atlas.textureId);
	}

	atlas		= other.atlas;
	characters	= std::move(other.characters);
	fontSize	= other.fontSize;

	other.atlas.textureId = TEXTURE_NOT_LOADED;

	return *this;
}

Font::~Font()
{
    if (atlas.textureId != TEXTURE_NOT_LOADED) {
        glDeleteTextures(1, &atlas.textureId);
    }
}

Font::Atlas const& Font::getAtlas() const
{
    return atlas;
}

unsigned int Font::getFontSize() const {
	return fontSize;
}

std::unordered_map<char, Font::Character> const& Font::getCharacters() const
{
    return characters;
}


#include "font.h"

#include <glad/glad.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Logger.h"

namespace {
    GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
    const int FONT_SIZE = 48;   // Default load size that all fonts use
}

Font::Font(ResourceID id, ResourceFilePath resourceFilePath) :
    Resource{ id, resourceFilePath },
    atlas{ TEXTURE_NOT_LOADED, 0, 0 }
{
    FT_Face face;
    FT_Library ft;
    // Initialize FreeType library
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    // Load font face using the stored fontPath
    if (FT_New_Face(ft, resourceFilePath.string.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << resourceFilePath.string << std::endl;
        return;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
    FT_GlyphSlot g = face->glyph;

    unsigned int w = 0;
    unsigned int h = 0;
    // First 32 chars for ascii are control characters
    for (unsigned char c = 32; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Atlas has every character in one line
        w += g->bitmap.width;
        h = std::max(h, g->bitmap.rows);
    }

    atlas.atlasWidth = w;
    atlas.atlasHeight = h;

    // generate texture
    GLuint atlasTex = 0;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int x = 0;
    for (unsigned char c = 32; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        Character character = {
            (float)x / w,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x >> 6
        };
        Characters.insert(std::pair<char, Character>(c, character));
        x += g->bitmap.width;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    atlas.atlasTextureID = atlasTex;

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Font::~Font()
{
    if (atlas.atlasTextureID != TEXTURE_NOT_LOADED) {
        glDeleteTextures(1, &atlas.atlasTextureID);
    }
}

Font::Font(Font && other) noexcept :
    Resource    { std::move(other) },
    atlas       { std::move(other.atlas)},
    Characters  { std::move(other.Characters) }
{
}

FRAMEWORK_DLL_API Font& Font::operator=(Font&& other) noexcept
{
    Resource::operator=(std::move(other));
    atlas = std::move(other.atlas);
    Characters = std::move(other.Characters);
    return *this;
}

FRAMEWORK_DLL_API int Font::getDefaultFontSize()
{
    return FONT_SIZE;
}

FRAMEWORK_DLL_API Font::Atlas Font::getAtlasDetails() const
{
    return atlas;
}

FRAMEWORK_DLL_API const std::map<char, Font::Character>& Font::getCharacters() const
{
    return Characters;
}


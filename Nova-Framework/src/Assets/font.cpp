#include "font.h"

#include <glad/glad.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Logger.h"

std::optional<Font> Font::LoadFont(const std::string& resourceFilePath)
{
    FT_Face face;
    FT_Library ft;
    // Initialize FreeType library
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return std::nullopt;
    }
    // Load font face using the stored fontPath
    if (FT_New_Face(ft, resourceFilePath.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << resourceFilePath << std::endl;
        return std::nullopt;
    }
    Font loadedFont;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    FT_Set_Pixel_Sizes(face, 0, loadedFont.FONT_SIZE);

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        loadedFont.Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return loadedFont;
}

FRAMEWORK_DLL_API int Font::GetDefaultFontSize()
{
    return FONT_SIZE;
}


//namespace {
//	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
//}
//
//Font::Font(ResourceID id, ResourceFilePath resourceFilePath) :
//	Resource		 { id, std::move(resourceFilePath) }
//{
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
//
//    for (unsigned char c = 0; c < 128; c++)
//    {
//        // load character glyph 
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//        {
//            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//            continue;
//        }
//        // generate texture
//        unsigned int texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//        // set texture options
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        // now store character for later use
//        Character character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            face->glyph->advance.x
//        };
//        Characters.insert(std::pair<char, Character>(c, character));
//    }
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//}
//
//Font::~Font() {
//}
//
//Font::Font(Font&& other) noexcept :
//	Resource		 { std::move(other) },
//    Characters       { std::move(other.Characters) }
//{
//}
//
//Font& Font::operator=(Font&& other) noexcept {
//	Resource::operator=(std::move(other));
//
//	Characters		 = std::move(other.Characters);
//
//	return *this;
//}
//
//GLuint Font::getFontId() const {
//	return textureId;
//}
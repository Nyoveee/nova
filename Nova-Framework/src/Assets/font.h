#pragma once

#include "resource.h"
#include <gli/gli.hpp>
#include <map>

using GLuint = unsigned int; 

class Font/* : public Resource */{
public:
	//FRAMEWORK_DLL_API Font(ResourceID id, ResourceFilePath resourceFilePath);
	//FRAMEWORK_DLL_API ~Font();

	//FRAMEWORK_DLL_API Font(Font const& other) = delete;
	//FRAMEWORK_DLL_API Font(Font&& other) noexcept;
	//FRAMEWORK_DLL_API Font& operator=(Font const& other) = delete;
	//FRAMEWORK_DLL_API Font& operator=(Font&& other) noexcept;
	//FRAMEWORK_DLL_API GLuint getFontId() const;
	FRAMEWORK_DLL_API static std::optional<Font> LoadFont(const std::string& resourceFilePath);
	FRAMEWORK_DLL_API int GetDefaultFontSize();
public:
	struct Character {
		GLuint textureID;  // ID handle of the glyph texture
		glm::ivec2   size;       // Size of glyph
		glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
		unsigned int advance;    // Offset to advance to next glyph
	};
	std::map<char, Character> Characters;
	// Default font size that is loaded into the program
	const int FONT_SIZE = 48;
};
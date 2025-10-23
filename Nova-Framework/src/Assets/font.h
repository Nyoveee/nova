#pragma once

#include "resource.h"
#include <gli/gli.hpp>
#include <map>

using GLuint = unsigned int; 

class Font : public Resource {
public:
	struct Atlas {
		GLuint atlasTextureID;  // the OpenGL texture handle
		GLuint atlasWidth;
		GLuint atlasHeight;
	};
	struct Character {
		float tx;				 // X offset of glyph in texture coords
		glm::ivec2   size;       // Size of glyph
		glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
		glm::uvec2	 advance;    // Offset to advance to next glyph
	};

	FRAMEWORK_DLL_API Font(ResourceID id, ResourceFilePath resourceFilePath);
	FRAMEWORK_DLL_API ~Font();

	FRAMEWORK_DLL_API Font(Font const& other) = delete;
	FRAMEWORK_DLL_API Font(Font&& other) noexcept;
	FRAMEWORK_DLL_API Font& operator=(Font const& other) = delete;
	FRAMEWORK_DLL_API Font& operator=(Font&& other) noexcept;
	FRAMEWORK_DLL_API int getDefaultFontSize();
	FRAMEWORK_DLL_API Atlas getAtlasDetails() const;
	FRAMEWORK_DLL_API const std::map<char, Character>& getCharacters() const;

	//FRAMEWORK_DLL_API static std::optional<Font> LoadFont(const std::string& resourceFilePath);
public:

private:
	Atlas atlas;
	std::map<char, Character> Characters;
};
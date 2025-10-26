#pragma once

#include "resource.h"
#include "texture.h"
#include <gli/gli.hpp>

using GLuint = unsigned int; 

class Font : public Resource {
public:
	struct Atlas {
		GLuint textureId;
		GLuint width;
		GLuint height;
	};

	struct Character {
		float tx;				 // X offset of glyph in texture coords
		glm::ivec2   size;       // Size of glyph
		glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
		int			 advance;    // Offset to advance to next glyph

		REFLECTABLE(
			tx,
			size,
			bearing,
			advance
		)
	};

	// this is the stuff that is de/serialized as a resource. the resource compiler compiles .ttf file to this.
	struct Data {
		// Represents a sub image of the texture atlas. Used soley to upload to OpenGL texture.
		struct Sprite {
			unsigned width;
			unsigned height;
			std::vector<unsigned char> bytes;

			REFLECTABLE(
				width,
				height,
				bytes
			)
		};
			
		std::vector<Sprite> sprites;
		std::unordered_map<char, Character> characters;

		unsigned int atlasWidth;
		unsigned int atlasHeight;
		unsigned int fontSize;

		REFLECTABLE(
			sprites,
			characters,
			atlasWidth,
			atlasHeight,
			fontSize
		)
	};

public:
	FRAMEWORK_DLL_API Font(ResourceID id, ResourceFilePath resourceFilePath, Data data);
	FRAMEWORK_DLL_API ~Font();

	FRAMEWORK_DLL_API Font(Font const& other) = delete;
	FRAMEWORK_DLL_API Font(Font&& other) noexcept;
	FRAMEWORK_DLL_API Font& operator=(Font const& other) = delete;
	FRAMEWORK_DLL_API Font& operator=(Font&& other) noexcept;

public:
	FRAMEWORK_DLL_API Atlas const& getAtlas() const;
	FRAMEWORK_DLL_API unsigned int getFontSize() const;
	FRAMEWORK_DLL_API std::unordered_map<char, Character> const& getCharacters() const;

private:
	Atlas atlas;
	std::unordered_map<char, Character> characters;
	unsigned int fontSize;
};
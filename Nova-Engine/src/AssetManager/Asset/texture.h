#pragma once

#include "asset.h"

using GLuint = unsigned int;

// 2 phase initialisation is generally frown upon in RAII practice, but in this case I may need to load and unload the texture often in a single lifetime.
// Similar to a default constructed std::ifstream not holding to any resource or std::function pointing to a nullptr

class Texture : public Asset {
public:
	DLL_API Texture(std::string filepath);
	DLL_API ~Texture();

	DLL_API Texture(Texture const& other) = delete;
	DLL_API Texture(Texture&& other) noexcept;
	DLL_API Texture& operator=(Texture const& other) = delete;
	DLL_API Texture& operator=(Texture&& other) noexcept;

public:
	DLL_API void load() final;
	DLL_API void unload() final;

public:
	DLL_API GLuint getTextureId() const;

private:
	GLuint textureId;
	int width;
	int height;
	int numChannels;
};
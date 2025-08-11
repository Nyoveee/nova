#pragma once

#include "asset.h"

using GLuint = unsigned int;

// 2 phase initialisation is generally frown upon in RAII practice, but in this case I may need to load and unload the texture often in a single lifetime.
// Similar to a default constructed std::ifstream not holding to any resource or std::function pointing to a nullptr

class Texture : public Asset {
public:
	Texture(std::string filepath);
	~Texture();

	Texture(Texture const& other) = delete;
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture const& other) = delete;
	Texture& operator=(Texture&& other) noexcept;

public:
	void load() final;
	void unload() final;

public:
	GLuint getTextureId() const;

private:
	GLuint textureId;
	int width;
	int height;
	int numChannels;
};
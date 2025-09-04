#include "audio.h"
#include "Libraries/stb_image.hpp"

#include <glad/glad.h>
#include <iostream>

#include "assetManager.h"
#include "Logger.h"

/*
Audio::Audio(std::string filepath) :
	Asset{ filepath },
	numChannels{}
{
}

Audio::Audio(Audio&& other) noexcept :
	Asset{ std::move(other) },
	numChannels{ other.numChannels }
{
}

Audio::~Audio() 
{
	if (!isLoaded()) {
		return;
	}
	unload();
}

Audio& Audio::operator=(Audio&& other) noexcept 
{
	Asset::operator=(std::move(other));

	if (isLoaded()) unload();
	numChannels = other.numChannels;
	return *this;
}

void Audio::load(AssetManager& assetManager) {
	// Left Blank as using audioSystem to load
}

void Audio::unload() {
	// Left Blank as using audioSystem to unload
}
*/
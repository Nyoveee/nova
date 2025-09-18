#include "audio.h"
#include <filesystem>

Audio::Audio(ResourceFilePath filepath, bool is3D) :
	Asset	{ std::move(filepath) },
	is3D	{ is3D }
{}

Audio::~Audio() {}

bool Audio::load() {
	return false;
}

void Audio::unload() {}

bool Audio::isAudio3D() const {
	return is3D;
}

std::string Audio::getClassName() const {
	return std::filesystem::path{ getFilePath() }.stem().string();
}

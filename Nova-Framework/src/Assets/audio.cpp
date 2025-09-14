#include "audio.h"
#include <filesystem>

Audio::Audio(std::string filepath, bool is3D) :
	Asset	{ filepath },
	is3D	{ is3D }
{}

Audio::~Audio() {}

void Audio::load() {
	Asset::loadStatus = Asset::LoadStatus::Loaded;
}

void Audio::unload() {}

bool Audio::isAudio3D() const {
	return is3D;
}

std::string Audio::getClassName() const {
	return std::filesystem::path{ getFilePath() }.stem().string();
}

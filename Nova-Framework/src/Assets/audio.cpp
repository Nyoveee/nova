#include "audio.h"
#include <filesystem>

Audio::Audio(ResourceID id, ResourceFilePath resourceFilePath, bool is3D) :
	Asset	{ id },
	is3D	{ is3D }
{}

Audio::~Audio() {}

bool Audio::isAudio3D() const {
	return is3D;
}

ResourceFilePath const& Audio::getFilePath() const {
	return resourceFilePath;
}
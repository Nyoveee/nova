#include "audio.h"
#include <filesystem>

Audio::Audio(ResourceID id, ResourceFilePath resourceFilePath, bool is3D) :
	Resource{ id, std::move(resourceFilePath) },
	is3D	{ is3D }
{}

Audio::~Audio() {}

bool Audio::isAudio3D() const {
	return is3D;
}
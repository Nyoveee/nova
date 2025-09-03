#include "audioSystem.h"
#include "Logger.h"

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

AudioSystem::AudioSystem(Engine& engine) :
	engine		{ engine },
	fmodSystem	{ nullptr }
{
	auto result = FMOD::System_Create(&fmodSystem);

	if (result != FMOD_OK) {
		Logger::error("Failed to create fmod system. {}", FMOD_ErrorString(result));
		return;
	}

	result = fmodSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	
	if (result != FMOD_OK) {
		Logger::error("Failed to initialise fmod system. {}", FMOD_ErrorString(result));
		return;
	}
}

AudioSystem::~AudioSystem() {
	if (fmodSystem) {
		fmodSystem->close();
		fmodSystem->release();
		fmodSystem = nullptr;
	}
}

void AudioSystem::update() {
	fmodSystem->update();
}

void AudioSystem::loadAllSounds() {
	Logger::info("Attempting to load all sounds");
	
	
}

void AudioSystem::unloadAllSounds() {
	Logger::info("Unload all sounds");

}

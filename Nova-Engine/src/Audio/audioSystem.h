#pragma once

#include <FMOD/fmod.hpp>
#include "export.h"
#include <unordered_map>
#include <string>

class Engine;

class AudioSystem {
public:
	DLL_API AudioSystem(Engine& engine);

	DLL_API ~AudioSystem();
	DLL_API AudioSystem(AudioSystem const& other)				= delete;
	DLL_API AudioSystem(AudioSystem&& other)					= delete;
	DLL_API AudioSystem& operator=(AudioSystem const& other)	= delete;
	DLL_API AudioSystem& operator=(AudioSystem&& other)			= delete;

public:
	void update();

public:
	void loadAllSounds();
	void unloadAllSounds();

	FMOD::Sound* getSound(const std::string& file);
	void stopAudio(const std::string& stringID);
    // PlaySFX based on string and assign a channelID and set the volume to global variable sfxVolume 
	DLL_API void playSFXInst(const std::string& soundID, float x, float y, float z);
	DLL_API	void playSFXNonInst(const std::string& soundID, float x, float y, float z);
	DLL_API void playBGM(const std::string& soundID);

private:
	std::string currentBGM = "";

	FMOD::System* fmodSystem;
	Engine& engine;
	std::unordered_map<std::string, FMOD::Sound*> sounds;
	std::unordered_map<std::string, std::string> soundFilePathMap;
	// Map soundID (string) to Channel ID (int)
	// Array of channels where the keys are std::string
	// Contains both SFX and BGM
	std::unordered_map<std::string, FMOD::Channel*> channels;

	// Store volume levels for each channel
	std::unordered_map<std::string, float> channelVolumes;
};
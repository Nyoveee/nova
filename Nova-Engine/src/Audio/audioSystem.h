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

private:
	FMOD::System* fmodSystem;
	Engine& engine;
	std::unordered_map<std::string, FMOD::Sound*> sounds;
	std::unordered_map<std::string, std::string> soundFilePathMap;
};
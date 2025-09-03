#pragma once

#include <FMOD/fmod.hpp>
#include "export.h"

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

private:

};
#pragma once

#include <FMOD/fmod.hpp>
#include "export.h"
#include <unordered_map>
#include <string>

#include "type_alias.h"

class Engine;
class AssetManager;

class AudioSystem {
public:
	struct AudioInstance {
		AudioInstanceID id;			// an id representing this audio instance
		AssetID audioId;			// holds an id back to the original audio file
		FMOD::Channel* channel;		// contains audio instance specific data 
		
		float volume = 1.f;
		bool toDelete = false;		// indicate that the audio instance should be deleted. should be used by callback only.
	};

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

	void stopAudioInstance(AudioInstance& audioInstance);
	void stopAudioInstance(AudioInstanceID audioInstanceId);

    // PlaySFX based on string and assign a channelID and set the volume to global variable sfxVolume 
	DLL_API void playSFX(AssetID audioId, float x, float y, float z);
	//DLL_API	void playSFXNonInst(AssetID audioId, float x, float y, float z);

	DLL_API void playBGM(AssetID audioId);

	void handleFinishedAudioInstance(FMOD::Channel* channel);

private:
	FMOD::Sound* getSound(AssetID audioId) const;
	void loadSound(AssetID audioId);

	AudioInstanceID getNewAudioInstanceId();
	AudioInstance* createSoundInstance(AssetID audioId, float volume = 1.f);

private:
	FMOD::System* fmodSystem;
	Engine& engine;
	AssetManager& assetManager;

private:
	AudioInstance* currentBGM;
	std::unordered_map<AssetID, FMOD::Sound*> sounds;

	AudioInstanceID nextAudioInstanceId = 0;

	//std::unordered_map<std::string, std::string> soundFilePathMap;
	
	// contains all playing sound instances..
	std::unordered_map<AudioInstanceID, AudioInstance> audioInstances;
};
#pragma once

#include <FMOD/fmod.hpp>
#include "export.h"
#include <unordered_map>
#include <string>

#include "type_alias.h"

class Engine;
class ResourceManager;

class AudioSystem {
public:
	struct AudioInstance {
		AudioInstanceID id;			// an id representing this audio instance
		ResourceID audioId;			// holds an id back to the original audio file
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
	float volCap = 2.0f;
	float sfxVolume;
	float sfxGlobal;  // stores sfxVolume * globalVolume

	float bgmVolume;
	float bgmGlobal;  // stores bgmVolume * globalVolume

	float globalVolume;
	float buttonVol;
	
	// For Screen Transitions
	// float prevSceneBGMVol;
	// float currBGMVol;
	// bool isFadingOut;
	// bool isFadingIn;

public:
	void update();
	void loadAllSounds();
	void unloadAllSounds();

	void stopAudioInstance(AudioInstance& audioInstance);
	void stopAudioInstance(AudioInstanceID audioInstanceId);

	// Retrieves resourceID from the unorderedmap using filename
	DLL_API ResourceID getResourceId(const std::string& string);

    // PlaySFX based on string and assign a channelID and set the volume to global variable sfxVolume 
	DLL_API void playSFX(ResourceID audioId, float x, float y, float z);
	//DLL_API	void playSFXNonInst(AssetID audioId, float x, float y, float z);

	// PlayBGM based on ResourceID audioId
	DLL_API void playBGM(ResourceID audioId);

	// Pause the sound of the sfx based on AssetID
	DLL_API void pauseSound(ResourceID audioId, bool paused);

	// Stops all currently playing audio files
	DLL_API void StopAllAudio();

	// Stops all currently playing audio files with AssetID audioId
	DLL_API void StopAudio(ResourceID audioId);

	DLL_API void AdjustVol(ResourceID audioId, float volume);

	DLL_API void AdjustGlobalVol(float volume);

	// Sets the game SFX volume level
	DLL_API void AdjustSFXVol(float volume);

	// Sets the game BGM volume level
	DLL_API void AdjustBGMVol(float volume);

	void handleFinishedAudioInstance(FMOD::Channel* channel);

private:
	FMOD::Sound* getSound(ResourceID audioId) const;
	void loadSound(ResourceID audioId);

	AudioInstanceID getNewAudioInstanceId();
	AudioInstance* createSoundInstance(ResourceID audioId, float volume = 1.f);

private:
	FMOD::System* fmodSystem;
	Engine& engine;
	ResourceManager& resourceManager;

private:
	AudioInstance* currentBGM;
	std::unordered_map<ResourceID, FMOD::Sound*> sounds;

	AudioInstanceID nextAudioInstanceId = 0;

	std::unordered_map<std::string, ResourceID> fileData;
	
	// contains all playing sound instances..
	std::unordered_map<AudioInstanceID, AudioInstance> audioInstances;
};
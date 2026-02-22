#pragma once

#include <entt/entt.hpp>
#include <FMOD/fmod.hpp>
#include "export.h"
#include <unordered_map>
#include <string>

#include "type_alias.h"
#include "audio.h"

struct AudioComponent;

class Engine;
class ResourceManager;

class AudioSystem {
public:
	struct AudioInstance {
		AudioInstanceID id;			// an id representing this audio instance
		ResourceID audioId;			// holds an id back to the original audio file
		FMOD::Channel* channel;		// contains audio instance specific data 
		entt::entity  entity = entt::null;
	
		float volume = 1.f;
		bool toDelete = false;		// indicate that the audio instance should be deleted. should be used by callback only.
	};

public:
	ENGINE_DLL_API AudioSystem(Engine& engine);
	ENGINE_DLL_API ~AudioSystem();
	ENGINE_DLL_API AudioSystem(AudioSystem const& other) = delete;
	ENGINE_DLL_API AudioSystem(AudioSystem&& other) = delete;
	ENGINE_DLL_API AudioSystem& operator=(AudioSystem const& other) = delete;
	ENGINE_DLL_API AudioSystem& operator=(AudioSystem&& other) = delete;

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

	// Positional Audio Functions
	void updateListener();
	void updatePositionalAudio();

	void stopAudioInstance(AudioInstance& audioInstance);
	void stopAudioInstance(AudioInstanceID audioInstanceId);

	// Retrieves resourceID from the unorderedmap using filename
	ENGINE_DLL_API ResourceID getResourceId(const std::string& string);
#if 0
	ENGINE_DLL_API void playSFX(ResourceID audioId, float x, float y, float z, float volume = 1.f);

	// PlayBGM based on ResourceID audioId
	ENGINE_DLL_API void playBGM(ResourceID audioId, float volume = 1.f);
#endif
	// Pause the sound of the sfx based on ResourceID audioId
	ENGINE_DLL_API void pauseSound(ResourceID audioId, bool paused);

	// Stops all currently playing audio files
	ENGINE_DLL_API void StopAllAudio();

	// Stops all currently playing audio files with ResourceID audioId
	ENGINE_DLL_API void StopAudio(entt::entity entity, ResourceID audioId);

	ENGINE_DLL_API void AdjustVol(ResourceID audioId, float volume);

	ENGINE_DLL_API void AdjustGlobalVol(float volume);

	// Sets the game SFX volume level
	ENGINE_DLL_API void AdjustSFXVol(float volume);

	// Sets the game BGM volume level
	ENGINE_DLL_API void AdjustBGMVol(float volume);

	void handleFinishedAudioInstance(FMOD::Channel* channel);

public:
	// These functions is called from the scripting API.
	ENGINE_DLL_API bool playSFX(entt::entity entity, AudioComponent const& audioComponent, TypedResourceID<Audio> audio);
	ENGINE_DLL_API bool playBGM(entt::entity entity, AudioComponent const& audioComponent, TypedResourceID<Audio> audio);
	ENGINE_DLL_API bool stopSound(entt::entity entity, TypedResourceID<Audio> audio);

	ENGINE_DLL_API void setMasterVolume(NormalizedFloat volume);
	ENGINE_DLL_API void setBGMVolume(NormalizedFloat volume);
	ENGINE_DLL_API void setSFXVolume(NormalizedFloat volume);

private:
	FMOD::Sound* getSound(ResourceID audioId) const;
	void loadSound(ResourceID audioId);

	AudioInstanceID getNewAudioInstanceId();
	AudioInstance* createSoundInstance(ResourceID audioId, AudioComponent const& audioComponent, entt::entity entity = entt::null);

private:
	FMOD::System* fmodSystem;
	Engine& engine;
	ResourceManager& resourceManager;

	FMOD::ChannelGroup* masterChannelGroup;
	FMOD::ChannelGroup* bgmChannelGroup;
	FMOD::ChannelGroup* sfxChannelGroup;

	std::unordered_map<ResourceID, FMOD::SoundGroup*> sfxSoundGroups;
private:
	AudioInstance* currentBGM;
	std::unordered_map<ResourceID, FMOD::Sound*> sounds;

	AudioInstanceID nextAudioInstanceId = 0;

	std::unordered_map<std::string, ResourceID> fileData;

	// contains all playing sound instances
	std::unordered_map<AudioInstanceID, AudioInstance> audioInstances;
};
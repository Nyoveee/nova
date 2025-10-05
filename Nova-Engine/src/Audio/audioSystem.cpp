#include "Audio/audioSystem.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/engine.h"
#include "Logger.h"
#include "audio.h"

#include "Profiling.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>

#undef max
#undef min

namespace {
	AudioSystem* g_audioSystem = nullptr;

	FMOD_RESULT channelCallback(
		FMOD_CHANNELCONTROL* channelcontrol,
		FMOD_CHANNELCONTROL_TYPE controltype,
		FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
		void* commanddata1,
		void* commanddata2
	) {
		(void)commanddata1;
		(void)commanddata2;

		if (callbacktype != FMOD_CHANNELCONTROL_CALLBACK_END) {
			return FMOD_OK;
		}

		switch (controltype) {
		case FMOD_CHANNELCONTROL_CHANNEL: {
			FMOD::Channel* channel = (FMOD::Channel*)channelcontrol;
			g_audioSystem->handleFinishedAudioInstance(channel);
			return FMOD_OK;
		}
		case FMOD_CHANNELCONTROL_CHANNELGROUP: {
			return FMOD_OK;
		}
		default:
			return FMOD_OK;
		}
	}
}

AudioSystem::AudioSystem(Engine& engine) :
	engine			{ engine },
	resourceManager	{ engine.resourceManager },
	fmodSystem		{ nullptr },
	currentBGM		{ nullptr }
{
	g_audioSystem = this;

	auto result = FMOD::System_Create(&fmodSystem);

	if (result != FMOD_OK) {
		Logger::error("Failed to create fmod system. {}", FMOD_ErrorString(result));
		return;
	}
    
    result = fmodSystem->init(4095, FMOD_INIT_NORMAL, nullptr);
    result = fmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);

    if (result != FMOD_OK) {
		Logger::error("Failed to initialise fmod system. {}", FMOD_ErrorString(result));
		return;
	}
	loadAllSounds();
}

AudioSystem::~AudioSystem() {
	unloadAllSounds();
	
	if (fmodSystem) {
		fmodSystem->close();
		fmodSystem->release();
		fmodSystem = nullptr;
	}
}

void AudioSystem::update() {
	ZoneScoped;

	fmodSystem->update();

	for (auto it = audioInstances.begin(); it != audioInstances.end();) {
		auto& [id, audioInstance] = *it;

		// Sound instance has expired, goodbye!
		if (audioInstance.toDelete) {
			it = audioInstances.erase(it);
			continue;
		}
		else {
			++it;
		}
	}
}

void AudioSystem::loadAllSounds() {
    Logger::info("Attempting to load all sounds");

    if (!fmodSystem)
    {
        Logger::info("Audio system is not initialized.");
        return;
    }

	// Get all audio assets.
	auto&& audios = resourceManager.getAllResources<Audio>();

	for (ResourceID audioId : audios) {
		loadSound(audioId);
	
		// Get the file name from the resource’s file path
		auto&& [audio, _] = resourceManager.getResource<Audio>(audioId);
		if (audio) {
			std::filesystem::path p(audio->getFilePath());
			std::string filename = p.stem().string(); // "BGM_AudioTest" from "BGM_AudioTest.wav"

			// Store into the map
			fileData[filename] = audioId;
		}
	}
}

void AudioSystem::unloadAllSounds() {
	Logger::info("Unloading All Sounds");

	currentBGM = nullptr;

	// Stop all audio instance.
	for (auto&& [audioInstanceId, audioInstance] : audioInstances) {
		audioInstance.channel->stop();
	}

	fileData.clear();
	audioInstances.clear();

	// Unload all sounds.
	for (auto&& [assetId, sound] : sounds) {
		if (sound) sound->release();
	}

	sounds.clear();

    Logger::info("All Sounds Unloaded");
}

void AudioSystem::stopAudioInstance(AudioInstance& audioInstance) {
	audioInstance.channel->stop();
	audioInstances.erase(audioInstance.id);
}

void AudioSystem::stopAudioInstance(AudioInstanceID audioInstanceId) {
	auto iterator = audioInstances.find(audioInstanceId);

	if (iterator == audioInstances.end()) {
		Logger::warn("Attempting to stop audio instance of invalid ID: {}", static_cast<std::size_t>(audioInstanceId));
		return;
	}

	auto&& [_, audioInstance] = *iterator;
	stopAudioInstance(audioInstance);
}

void AudioSystem::playSFX(entt::entity entity, std::string soundName) {
	AudioComponent* audio = engine.ecs.registry.try_get<AudioComponent>(entity);

	if (!audio) {
		Logger::warn("Attempting to play sound from entity with no audio component!");
		return;
	}

	auto iterator = audio->data.find(soundName);

	if (iterator == audio->data.end()) {
		Logger::warn("Entity has no sound named {}", soundName);
		return;
	}

	Transform const& transform = engine.ecs.registry.get<Transform>(entity);

	auto&& [_, audioData] = *iterator;
	playSFX(audioData.AudioId, transform.position.x, transform.position.y, transform.position.z, audioData.Volume);
}

FMOD::Sound* AudioSystem::getSound(ResourceID audioId) const {
	auto iterator = sounds.find(audioId);

	if (iterator == sounds.end()) {
		return nullptr;
	}

	auto&& [_, sound] = *iterator;
	return sound;
}

ResourceID AudioSystem::getResourceId(const std::string& string) {
	auto it = fileData.find(string);
	if (it != fileData.end()) {
		return it->second;
	}

	return INVALID_RESOURCE_ID;
}

// PlaySFX based on string and assign a channelID and set the volume to global variable sfxVolume 
void AudioSystem::playSFX(ResourceID id, float x, float y, float z, float volume )
{
    // Play the sound
	AudioInstance* audioInstance = createSoundInstance(id , volume );

	if (!audioInstance) {
		return;
	}

	FMOD_VECTOR position = { x, y, z };
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

	audioInstance->channel->set3DAttributes(&position, &velocity);
	audioInstance->channel->setPaused(false);
}

void AudioSystem::playBGM(ResourceID id, float volume)
{
    // Stop previous BGM.
	if (currentBGM) {
		stopAudioInstance(*currentBGM);
		currentBGM = nullptr;
	}

	AudioInstance* audioInstance = createSoundInstance(id , volume);

    // Update current BGM
	if (audioInstance) {
		currentBGM = audioInstance;
	}
}

void AudioSystem::pauseSound(ResourceID audioId, bool paused)
{
	// Find all instances that were created from this audioId
	for (auto& [id, audioInstance] : audioInstances)
	{
		if (audioInstance.audioId == audioId && audioInstance.channel)
		{
			audioInstance.channel->setPaused(paused);
		}
	}
}

void AudioSystem::StopAllAudio()
{
	for (auto& [instanceId, audioInstance] : audioInstances)
	{
		if (audioInstance.channel)
		{
			audioInstance.channel->stop();
		}
	}

	audioInstances.clear();
	currentBGM = nullptr;
}

void AudioSystem::StopAudio(ResourceID audioId)
{
	for (auto it = audioInstances.begin(); it != audioInstances.end(); )
	{
		auto& [instanceId, audioInstance] = *it;

		if (audioInstance.audioId == audioId && audioInstance.channel)
		{
			audioInstance.channel->stop();
			it = audioInstances.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (currentBGM && currentBGM->audioId == audioId)
	{
		currentBGM = nullptr;
	}
}

void AudioSystem::AdjustVol(ResourceID audioId, float volume)
{
	for (auto& [instanceId, audioInstance] : audioInstances)
	{
		if (audioInstance.audioId == audioId && audioInstance.channel)
		{
			audioInstance.channel->setVolume(volume);
			audioInstance.volume = volume;
		}
	}

	if (currentBGM && currentBGM->audioId == audioId && currentBGM->channel)
	{
		currentBGM->channel->setVolume(volume);
		currentBGM->volume = volume;
	}
}

// ** Changes the Overall game volume level **
void AudioSystem::AdjustGlobalVol(float volume)
{
	globalVolume = volume;
	AudioSystem::AdjustSFXVol(sfxVolume);
	AudioSystem::AdjustSFXVol(bgmVolume);
}

// Sets the game SFX volume level
void AudioSystem::AdjustSFXVol(float volume)
{
	sfxVolume = volume;

	if (globalVolume <= 0) 
	{
		sfxGlobal = 0.0f;
		buttonVol = sfxGlobal;
	} else {
		sfxGlobal = std::min(sfxVolume * globalVolume, volCap);
		buttonVol = sfxGlobal;
	}
}

// Sets the game BGM volume level
void AudioSystem::AdjustBGMVol(float volume)
{
	bgmVolume = volume;
	bgmGlobal = std::min(bgmVolume * globalVolume, volCap);

	if (currentBGM && currentBGM->channel)
	{
		currentBGM->channel->setVolume(volume);
		currentBGM->volume = volume;
	}
}

void AudioSystem::handleFinishedAudioInstance(FMOD::Channel* channel) {
	auto iterator = std::find_if(audioInstances.begin(), audioInstances.end(), [&](auto const& keyPairValue) {
		return channel == keyPairValue.second.channel;
	});

	if (iterator == audioInstances.end()) {
		return;
	}

	//stopAudioInstance(iterator->first);
	iterator->second.toDelete = true;
}

void AudioSystem::loadSound(ResourceID audioId) {
	auto&& [audio, _] = resourceManager.getResource<Audio>(audioId);

	if (!audio) {
		Logger::error("Invalid audio id when loading sound: {}. This should not have happened.", static_cast<std::size_t>(audioId));
		return;
	}

	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = audio->isAudio3D() ? FMOD_3D : FMOD_2D;

	FMOD_RESULT result = fmodSystem->createSound(audio->getFilePath().string.c_str(), mode, nullptr, &sound);

	if (result != FMOD_OK) {
	    Logger::warn("Failed to load audio file with asset id of: {}, filepath of {}.", static_cast<std::size_t>(audioId), audio->getFilePath().string);
	    return; 
	}

	sounds[audioId] = sound;

	// Configure 3D properties
	if (audio->isAudio3D()) {
	    sound->set3DMinMaxDistance(100.0f, 200.0f);
	}
}

AudioInstanceID AudioSystem::getNewAudioInstanceId() {
	AudioInstanceID idToReturn = nextAudioInstanceId;
	nextAudioInstanceId = static_cast<std::size_t>(nextAudioInstanceId) + 1ULL;
	return idToReturn;
}

AudioSystem::AudioInstance* AudioSystem::createSoundInstance(ResourceID audioId, float volume) {
	FMOD::Sound* audio = AudioSystem::getSound(audioId);
	
	if (!audio) {
		Logger::info("Sound not found: {}", static_cast<std::size_t>(audioId));
		return nullptr;
	}

	FMOD::Channel* channel = nullptr;
	fmodSystem->playSound(audio, nullptr, false, &channel);

	if (channel) {
		AudioInstanceID	audioInstanceId = getNewAudioInstanceId();
		AudioInstance& audioInstance = audioInstances[audioInstanceId];

		audioInstance = { audioInstanceId, audioId, channel, volume };
		audioInstance.channel->setVolume(audioInstance.volume);
		audioInstance.channel->setCallback(channelCallback);

		return &audioInstance;
	}
	else {
		Logger::warn("Failed to create sound instance with audioId: {}", static_cast<std::size_t>(audioId));
		return nullptr;
	}
}

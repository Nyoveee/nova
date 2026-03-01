#include "Audio/audioSystem.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/engine.h"
#include "Logger.h"
#include "audio.h"
#include "Graphics/camera.h"
#include "Graphics/renderer.h"
#include "component.h"

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

	fmodSystem->createChannelGroup("Master", &masterChannelGroup);
	fmodSystem->createChannelGroup("BGM", &bgmChannelGroup);
	fmodSystem->createChannelGroup("SFX", &sfxChannelGroup);

	masterChannelGroup->addGroup(bgmChannelGroup);
	masterChannelGroup->addGroup(sfxChannelGroup);

	// Set initial audio group..
	masterChannelGroup->setVolume(engine.dataManager.audioConfig.masterVolume);
	bgmChannelGroup->setVolume(engine.dataManager.audioConfig.bgmVolume);
	sfxChannelGroup->setVolume(engine.dataManager.audioConfig.sfxVolume);
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
#if !defined(NOVA_INSTALLER)
	ZoneScoped;
#endif

	// Update listener position based on camera
	updateListener();

	// Update positional audio sources
	updatePositionalAudio();

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

	for (auto positionalAudioGroupsIter = positionalAudioGroups.begin(); positionalAudioGroupsIter != positionalAudioGroups.end(); ++positionalAudioGroupsIter) {
		auto& [resourceID, positionalAudioInstances] = *positionalAudioGroupsIter;
		if (positionalAudioInstances.empty())
			continue;
		auto todelete = [&](const AudioInstance& audioInstance) {
			return engine.ecs.registry.valid(audioInstance.entity) || audioInstance.toDelete;
		};

		auto audioInstancesIter = std::remove_if(positionalAudioInstances.begin(), positionalAudioInstances.end(), todelete);
		if(audioInstancesIter!= positionalAudioInstances.end())
			positionalAudioInstances.erase(audioInstancesIter, positionalAudioInstances.end());
	}
}

void AudioSystem::updateListener() {
	// Get camera position and orientation
	Camera const& camera	= engine.renderer.getGameCamera();
	glm::vec3 listenerPos	= camera.getPos();
	glm::vec3 listenerFront = camera.getFront();
	glm::vec3 listenerRight = camera.getRight();
	
	listenerFront = glm::normalize(listenerFront);
	listenerRight = glm::normalize(listenerRight);

	// Calculate up vector (perpendicular to front and right)
	glm::vec3 listenerUp = glm::normalize(glm::cross(listenerFront, listenerRight));

	// Set FMOD listener position and orientation
	FMOD_VECTOR pos = { listenerPos.x, listenerPos.y, listenerPos.z };
	FMOD_VECTOR forward = { listenerFront.x, listenerFront.y, listenerFront.z };
	FMOD_VECTOR up = { listenerUp.x, listenerUp.y, listenerUp.z };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f }; 

	fmodSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
}

void AudioSystem::updatePositionalAudio() {
	// Get camera position for distance calculations
	Camera const& camera = engine.renderer.getGameCamera();
	glm::vec3 listenerPos = camera.getPos();

	// Get all objects with PositionalAudio Component
	for (auto positionalAudioGroupsIter = positionalAudioGroups.begin(); positionalAudioGroupsIter != positionalAudioGroups.end(); ++positionalAudioGroupsIter) {
		auto& [resourceID, positionalAudioInstances] = *positionalAudioGroupsIter;
		if (positionalAudioInstances.empty())
			continue;
		
		// Sort based on Distance
		auto comp = [&](AudioInstance const& audioInstance1, AudioInstance const& audioInstance2) {
			Transform& transform1 = engine.ecs.registry.get<Transform>(audioInstance1.entity);
			Transform& transform2 = engine.ecs.registry.get<Transform>(audioInstance2.entity);
			float distance1 = glm::length(transform1.position - listenerPos);
			float distance2 = glm::length(transform2.position - listenerPos);
			return distance1 < distance2;
		};
		std::sort(std::begin(positionalAudioInstances), std::end(positionalAudioInstances), comp);
		// Set Volume
		for (int i{}; i < positionalAudioInstances.size();++i) {
			Transform* transform = engine.ecs.registry.try_get<Transform>(positionalAudioInstances[i].entity);
			PositionalAudio* positionalAudio = engine.ecs.registry.try_get<PositionalAudio>(positionalAudioInstances[i].entity);

			if (!transform || !positionalAudio) {
				continue;
			}

			glm::vec3 sourcePos = transform->position;
			float distance = glm::length(sourcePos - listenerPos);
			float volumeMultiplier = 1.0f;
			if (distance <= positionalAudio->innerRadius)
				volumeMultiplier = 1.0f;
			else if (distance >= positionalAudio->maxRadius)
				volumeMultiplier = 0.0f;
			else
				volumeMultiplier = 1.0f - ((distance - positionalAudio->innerRadius) / (positionalAudio->maxRadius - positionalAudio->innerRadius));


			if (positionalAudioInstances[i].channel) {
				FMOD_VECTOR pos = { sourcePos.x, sourcePos.y, sourcePos.z };
				FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

				positionalAudioInstances[i].channel->set3DAttributes(&pos, &vel);
				// Set the MinMax Distance based on the values inputted inside the PositionalAudio Component inside the Editor 
				positionalAudioInstances[i].channel->set3DMinMaxDistance(positionalAudio->innerRadius, positionalAudio->maxRadius);
				float zipfMultiplier = 1.f / (static_cast<float>(i) + 1.f);
				positionalAudioInstances[i].channel->setVolume(positionalAudioInstances[i].volume * volumeMultiplier * zipfMultiplier);
			}
		}
	}
}

void AudioSystem::loadAllSounds() {
	Logger::debug("Attempting to load all sounds");

	if (!fmodSystem)
	{
		Logger::debug("Audio system is not initialized.");
		return;
	}

	// Get all audio assets.
	auto&& audios = resourceManager.getAllResources<Audio>();

	for (ResourceID audioId : audios) {
		loadSound(audioId);

		// Get the file name from the resources file path
		auto&& [audio, _] = resourceManager.getResource<Audio>(audioId);
		if (audio) {
			std::filesystem::path p(audio->getFilePath());
			std::string filename = p.stem().string();

			// Store into the map
			fileData[filename] = audioId;
		}
	}
}

void AudioSystem::unloadAllSounds() {
	Logger::debug("Unloading All Sounds");

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

	Logger::debug("All Sounds Unloaded");
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
#if 0
void AudioSystem::playSFX(ResourceID id, float x, float y, float z, float volume)
{
	// Play the sound
	AudioInstance* audioInstance = createSoundInstance(id, volume);

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

	AudioInstance* audioInstance = createSoundInstance(id, volume);
	// Update current BGM
	if (audioInstance) {
		currentBGM = audioInstance;
	}
}
#endif
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

void AudioSystem::StopAudio(entt::entity entity, ResourceID audioId)
{
	for (auto it = audioInstances.begin(); it != audioInstances.end(); )
	{
		auto& [instanceId, audioInstance] = *it;

		if (audioInstance.entity == entity && audioInstance.audioId == audioId && audioInstance.channel)
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
	}
	else {
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
	FMOD_RESULT result = fmodSystem->createSound(audio->getFilePath().string.c_str(), FMOD_DEFAULT, nullptr, &sound);

	if (result != FMOD_OK) {
		Logger::warn("Failed to load audio file with asset id of: {}, filepath of {}.", static_cast<std::size_t>(audioId), audio->getFilePath().string);
		return;
	}

	sounds[audioId] = sound;
}

AudioInstanceID AudioSystem::getNewAudioInstanceId() {
	AudioInstanceID idToReturn = nextAudioInstanceId;
	nextAudioInstanceId = static_cast<std::size_t>(nextAudioInstanceId) + 1ULL;
	return idToReturn;
}

AudioSystem::AudioInstance* AudioSystem::createSoundInstance(ResourceID audioId, AudioComponent const& audioComponent, entt::entity entity ) {
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
		PositionalAudio * positionalAudio = engine.ecs.registry.try_get<PositionalAudio>(entity);

		audioInstance = { audioInstanceId, audioId, channel, entity , audioComponent.volume };
		audioInstance.channel->setVolume(audioInstance.volume);
		audioInstance.channel->setCallback(channelCallback);

		audioInstance.channel->setMode((positionalAudio ? FMOD_3D : FMOD_2D) | (audioComponent.loop? FMOD_LOOP_NORMAL : FMOD_DEFAULT));

		// assign to proper sound group..
		switch (audioComponent.audioGroup) {
		case AudioComponent::AudioGroup::BGM:
			channel->setChannelGroup(bgmChannelGroup);
			break;
		case AudioComponent::AudioGroup::SFX:
		{
			channel->setChannelGroup(sfxChannelGroup);
			if(positionalAudio)
				positionalAudioGroups[audioInstance.audioId].push_back(audioInstance);
			break;
		}
		default:
			channel->setChannelGroup(masterChannelGroup);
			break;
		}

		return &audioInstance;
	}
	else {
		Logger::warn("Failed to create sound instance with audioId: {}", static_cast<std::size_t>(audioId));
		return nullptr;
	}
}

/***********************************************************************************************************
	Scripting Functions
***********************************************************************************************************/
bool AudioSystem::playBGM(entt::entity entity, AudioComponent const& audioComponent, TypedResourceID<Audio> audio)
{

	if (audio == INVALID_RESOURCE_ID)
		return false;

	// Stop previous BGM.
	if (currentBGM) {
		stopAudioInstance(*currentBGM);
		currentBGM = nullptr;
	}

	AudioInstance* audioInstance = createSoundInstance(audio, audioComponent, entity);
	// Update current BGM
	if (audioInstance) {
		currentBGM = audioInstance;
	}
	return true;

}
bool AudioSystem::playSFX(entt::entity entity, AudioComponent const& audioComponent, TypedResourceID<Audio> audio)
{
	if (audio == INVALID_RESOURCE_ID)
		return false;

	Transform const& transform = engine.ecs.registry.get<Transform>(entity);

	AudioInstance* audioInstance = createSoundInstance(audio, audioComponent, entity);
	if (!audioInstance) 
		return false;

	FMOD_VECTOR pos = { transform.position.x, transform.position.y, transform.position.z };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	audioInstance->channel->set3DAttributes(&pos, &vel);

	// Apply per-entity attenuation
	if (auto* positional = engine.ecs.registry.try_get<PositionalAudio>(entity))
	{
		audioInstance->channel->set3DMinMaxDistance(positional->innerRadius, positional->maxRadius);
	}

	audioInstance->channel->setPaused(false);
	return true;
}
bool AudioSystem::stopSound(entt::entity entity, TypedResourceID<Audio> audio)
{
	if (audio == INVALID_RESOURCE_ID)
		return false;
	StopAudio(entity, audio);
	return true;
}

void AudioSystem::setMasterVolume(NormalizedFloat volume) {
	masterChannelGroup->setVolume(volume);
}

void AudioSystem::setBGMVolume(NormalizedFloat volume) {
	bgmChannelGroup->setVolume(volume);
}

void AudioSystem::setSFXVolume(NormalizedFloat volume) {
	sfxChannelGroup->setVolume(volume);
}
void AudioSystem::onEnginePaused() {
	sfxChannelGroup->setPaused(true);
	//for (auto& [instanceId, audioInstance] : audioInstances) {
	//	if (!audioInstance.channel) continue;
	//
	//	// Check if the entity has an AudioComponent with playWhenPaused set
	//	bool shouldKeepPlaying = false;
	//	if (audioInstance.entity != entt::null) {
	//		if (auto* comp = engine.ecs.registry.try_get<AudioComponent>(audioInstance.entity)) {
	//			shouldKeepPlaying = comp->playWhenPaused;
	//		}
	//	}
	//
	//	if (!shouldKeepPlaying) {
	//		audioInstance.channel->setPaused(true);
	//	}
	//}
}

void AudioSystem::onEngineResumed() {
	sfxChannelGroup->setPaused(false);
	//for (auto& [instanceId, audioInstance] : audioInstances) {
	//	if (!audioInstance.channel) continue;
	//
	//	bool shouldKeepPlaying = false;
	//	if (audioInstance.entity != entt::null) {
	//		if (auto* comp = engine.ecs.registry.try_get<AudioComponent>(audioInstance.entity)) {
	//			shouldKeepPlaying = comp->playWhenPaused;
	//		}
	//	}
	//
	//	// Only resume sounds that were paused by the engine pause (not manually paused)
	//	if (!shouldKeepPlaying) {
	//		audioInstance.channel->setPaused(false);
	//	}
	//}
}

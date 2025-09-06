#include "audioSystem.h"
#include "Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>
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
    
    result = fmodSystem->init(4095, FMOD_INIT_NORMAL, nullptr);
    result = fmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);

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

    if (!fmodSystem)
    {
        Logger::info("Audio system is not initialized.");
        return;
    }

    std::string fileName{ "Assets/Audio/AudioFilesToRead.txt" };
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cout << "Error: Txt File: " << fileName << " could not be opened\n";
        return;
    }

    int counter = 1;

    while (!file.eof())
    {
        std::string to3D;
        bool loop;
        std::string wavPath;

        file >> to3D >> loop >> wavPath;

        if (file.fail()) {
            break;
        }

        std::string soundName{ std::filesystem::path(wavPath).filename().string() };
        Logger::info("Audio File " + std::to_string(counter) + ". Loading " + soundName);

        // Determine sound mode
        FMOD_MODE mode = FMOD_DEFAULT | ((to3D == "3D") ? FMOD_3D : FMOD_2D);
        if (loop) {
            mode |= FMOD_LOOP_NORMAL;
        }

        FMOD::Sound* audio = nullptr;

        auto result = fmodSystem->createSound(wavPath.c_str(), mode, nullptr, &audio);
        if (result != FMOD_OK)
        {
            Logger::info("Audio File " + std::to_string(counter) + ". " + soundName + " [Failed]");
            counter++;
            continue; 
        }

        // Configure 3D properties
        if (to3D == "3D" && audio)
        {
            audio->set3DMinMaxDistance(100.0f, 200.0f);
        }

        Logger::info("Audio File " + std::to_string(counter) + ". " + soundName + " [Success]");
        
        counter++;

        // Store the sound
        sounds[soundName] = audio;
        soundFilePathMap[wavPath] = soundName;
    }

    file.close();
}

void AudioSystem::unloadAllSounds() {
	Logger::info("Unloading All Sounds");

    for (auto& pair : sounds)
    {
        if (pair.second != nullptr) {
            pair.second->release();  // Release the sound to free resources
        }
    }

    // Clear the unordered maps
    sounds.clear();
    soundFilePathMap.clear();

    Logger::info("All Sounds Unloaded");
}

FMOD::Sound* AudioSystem::getSound(const std::string& file) 
{
    return sounds[file];
}

void AudioSystem::stopAudio(const std::string& stringID)
{
    for (auto& [soundID, channel] : channels) {
        if (channel && soundID == stringID) {          // Ensure the channel pointer is valid
            channel->stop();                           // Stops the channel
        }
    }
}

// PlaySFX based on string and assign a channelID and set the volume to global variable sfxVolume 
void AudioSystem::playSFXInst(const std::string& soundID, float x, float y, float z)
{

    FMOD::Sound* audio = AudioSystem::getSound(soundID);
    if (!audio) {
        Logger::info("Sound not found: " + soundID);
        return;
    }

    FMOD_MODE mode{};
    audio->getMode(&mode);

    // Play the sound
    fmodSystem->playSound(audio, nullptr, false, &channels[soundID]);

    if (channels[soundID]) {
        FMOD_VECTOR position = { x, y, z };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

        channels[soundID]->set3DAttributes(&position, &velocity);

        float volume = 1.0f;
        if (channelVolumes.contains(soundID)) {
            volume = channelVolumes[soundID];
        }

        channels[soundID]->setVolume(volume);
        channels[soundID]->setPaused(false);

    }
}

void AudioSystem::playSFXNonInst(const std::string& soundID, float x, float y, float z)
{
    FMOD::Sound* audio = getSound(soundID);
    if (!audio) {
        Logger::info("Sound not found: " + soundID);
        return;
    }

    // Check if the sound already has a channel
    auto it = channels.find(soundID);
    if (it != channels.end() && it->second) {
        bool isPlaying = false;
        it->second->isPlaying(&isPlaying);

        if (isPlaying) {
            return;
        }
    }

    FMOD::Channel* channel = nullptr;
    fmodSystem->playSound(audio, nullptr, false, &channel);

    if (channel) {
        FMOD_VECTOR position = { x, y, z };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

        channel->set3DAttributes(&position, &velocity);

        float volume = 1.0f;
        if (channelVolumes.contains(soundID)) {
            volume = channelVolumes[soundID];
        }
        channel->setVolume(volume);
        channel->setPaused(false);

        // Store channel for reuse
        channels[soundID] = channel;
    }
}

void AudioSystem::playBGM(const std::string& soundID)
{
    // Stop previous BGM if different
    if (!currentBGM.empty() && currentBGM != soundID) {
        stopAudio(currentBGM);
    }

    // Update current BGM
    currentBGM = soundID;

    // Get the sound
    FMOD::Sound* sound = getSound(soundID);
    if (!sound) {
        Logger::info("Audio File " + currentBGM + " Loading [Failed]");
        return;
    }

    // Check if it's already playing
    bool isPlaying = false;
    auto it = channels.find(soundID);
    if (it != channels.end() && it->second) {
        it->second->isPlaying(&isPlaying);
    }

    if (isPlaying) {
        Logger::info(soundID + " is currently playing");
        return;
    }

    // Play the new BGM
    auto result = fmodSystem->playSound(sound, nullptr, false, &channels[soundID]);
    if (result != FMOD_OK) {
        Logger::info("FMOD playSound failed: " + currentBGM + " unable to be played");
        return;
    }

    // Set volume
    if (channels[soundID]) {
        channels[soundID]->setVolume(1.0f);
    }
}
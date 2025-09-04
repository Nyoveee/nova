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
        Logger::info("Loading Audio File " + std::to_string(counter) + ". " + soundName);

        // Determine sound mode
        FMOD_MODE mode = FMOD_DEFAULT | ((to3D == "3D") ? FMOD_3D : FMOD_2D);
        if (loop) {
            mode |= FMOD_LOOP_NORMAL;
        }

        FMOD::Sound* audio = nullptr;

        if (fmodSystem->createSound(wavPath.c_str(), mode, nullptr, &audio) != FMOD_OK)
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

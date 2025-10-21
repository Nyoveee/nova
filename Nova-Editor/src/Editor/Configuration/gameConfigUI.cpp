#include "gameConfigUI.h"
#include "imgui.h"
#include <fstream>
#include <json/json.hpp>
#include <iostream>

using json = nlohmann::json;

GameConfigUI::GameConfigUI(Editor& editor) :
    editor{ editor }
{
    loadConfig();
}

void GameConfigUI::update() {
    ImGui::Begin("Game Configuration");

    if (ImGui::CollapsingHeader("Window Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Window name input
        ImGui::Text("Window Name");
        static char nameBuffer[256];
        strcpy_s(nameBuffer, windowName.c_str()); 
        if (ImGui::InputText("##WindowName", nameBuffer, sizeof(nameBuffer))) {
            windowName = std::string(nameBuffer);
        }

        // Window size inputs
        ImGui::Text("Game Width");
        ImGui::InputInt("##GameWidth", &gameWidth);
        if (gameWidth < 1) gameWidth = 1;

        ImGui::Text("Game Height");
        ImGui::InputInt("##GameHeight", &gameHeight);
        if (gameHeight < 1) gameHeight = 1;
    }

    ImGui::Separator();

    // Action buttons
    if (ImGui::Button("Save")) {
        saveConfig();
    }

    ImGui::SameLine();

    if (ImGui::Button("Apply")) {
        applyConfig();
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset")) {
        windowName = "Nova Game";
        gameWidth = 1920;
        gameHeight = 1080;
        saveConfig();
    }

    ImGui::End();
}

void GameConfigUI::loadConfig() {
    try {
        std::ifstream file(configPath);
        if (file.good()) {
           // std::cout << "Game config file found: " << configPath << std::endl;
            json config = json::parse(file);

            if (config.contains("Window")) {
                auto& windowConfig = config["Window"];

                if (windowConfig.contains("gameWidth")) {
                    gameWidth = windowConfig["gameWidth"];
                  //  std::cout << "Loaded game width: " << gameWidth << std::endl;
                }

                if (windowConfig.contains("gameHeight")) {
                    gameHeight = windowConfig["gameHeight"];
                  //  std::cout << "Loaded game height: " << gameHeight << std::endl;
                }

                if (windowConfig.contains("windowName")) {
                    windowName = windowConfig["windowName"];
                  //  std::cout << "Loaded window name: " << windowName << std::endl;
                }
            }
            else {
             //   std::cout << "Config file exists but no Window settings found" << std::endl;
            }
        }
        else {
            std::cout << "Game config file not found: " << configPath << std::endl;
            // Create default config if file doesn't exist
            saveConfig();
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error loading game config: " << e.what() << std::endl;
        // Reset to defaults on error
        windowName = "Nova Game";
        gameWidth = 1920;
        gameHeight = 1080;
    }
}

void GameConfigUI::saveConfig() {
    try {
        json config;

        // Load existing config to preserve other settings 
        std::ifstream inputFile(configPath);
        if (inputFile.good()) {
            try {
                config = json::parse(inputFile);
                std::cout << "Loaded existing game config file" << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << "Error parsing existing game config, creating new one: " << e.what() << std::endl;
            }
        }
        else {
           std::cout << "No existing game config file found, creating new one" << std::endl;
        }

        // Update window settings in config
        config["Window"]["gameWidth"] = gameWidth;
        config["Window"]["gameHeight"] = gameHeight;
        config["Window"]["windowName"] = windowName;

        /*
        std::cout << "Saving game config - Window: " << windowName
            << " (" << gameWidth << "x" << gameHeight << ")" << std::endl;*/

        // Save back to file
        std::ofstream outputFile(configPath);
        if (outputFile.is_open()) {
            outputFile << config.dump(4); // keep 4 spaces indent
            outputFile.close();
           std::cout << "Successfully saved game config to: " << configPath << std::endl;
        }
        else {
           std::cout << "Failed to open file for writing: " << configPath << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Exception in saveConfig: " << e.what() << std::endl;
    }
}

void GameConfigUI::applyConfig() {
    // Need to apply the configuration to your game
    /*
    std::cout << "Applying game configuration - Window: " << windowName
        << " (" << gameWidth << "x" << gameHeight << ")" << std::endl;*/

}
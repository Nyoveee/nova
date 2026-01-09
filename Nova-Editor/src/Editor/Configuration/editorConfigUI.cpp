#include "editorConfigUI.h"
#include "imgui.h"
#include <fstream>
#include <json/json.hpp>
#include "iostream"

using json = nlohmann::ordered_json;

EditorConfigUI::EditorConfigUI(Editor& editor) :
    editor{ editor }
{
    loadConfig();
}

void EditorConfigUI::update() {
    ImGui::Begin("Editor Configuration");

    // Font size slider
    ImGui::Text("Font Size");
    if (ImGui::SliderInt("##FontSize", &fontSize, 8, 24)) {
       
    }

    ImGui::SameLine();
    if (ImGui::Button("Apply")) {
        // Apply the new font size and save configuration
        saveConfig();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        fontSize = 13;
        saveConfig();
    }

    ImGui::End();
}

void EditorConfigUI::loadConfig() {
    try {
        std::ifstream file(configPath);
        if (file.good()) {
          //  std::cout << "Config file found: " << configPath << std::endl;
            json config = json::parse(file);

            if (config.contains("Settings") &&
                config["Settings"].contains("font-size")) {
                fontSize = config["Settings"]["font-size"];
              //  std::cout << "Loaded font size: " << fontSize << std::endl;
            }
            else {
              //  std::cout << "Config file exists but no font-size setting found" << std::endl;
            }
        }
        else {
         //   std::cout << "Config file not found: " << configPath << std::endl;
        }
    }
    catch (const std::exception&) {
     //   std::cout << "Error loading config: " << e.what() << std::endl;
        fontSize = 13;
    }
}

void EditorConfigUI::saveConfig() {
    try {
        json config;

        // Load existing config to preserve other settings
        std::ifstream inputFile(configPath);
        if (inputFile.good()) {
            try {
                config = json::parse(inputFile);
              //  std::cout << "Loaded existing config file" << std::endl;
            }
            catch (const std::exception&) {
               // std::cout << "Error parsing existing config, creating new one: " << e.what() << std::endl;
            }
        }
        else {
           // std::cout << "No existing config file found, creating new one" << std::endl;
        }

        // Update font size in config
        config["Settings"]["font-size"] = fontSize;
     //   std::cout << "Setting font-size to: " << fontSize << std::endl;

        // Save back to file
        std::ofstream outputFile(configPath);
        if (outputFile.is_open()) {
            outputFile << config.dump(4); // Pretty print with 4 spaces indent
            outputFile.close();
           // std::cout << "Successfully saved config to: " << configPath << std::endl;
          //  std::cout << "File content: " << config.dump(4) << std::endl;
        }
        else {
          //  std::cout << "Failed to open file for writing: " << configPath << std::endl;
        }
    }
    catch (const std::exception&) {
       // std::cout << "Exception in saveConfig: " << e.what() << std::endl;
    }
}
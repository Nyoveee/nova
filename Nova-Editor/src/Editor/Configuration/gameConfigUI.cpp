#include "gameConfigUI.h"
#include "imgui.h"
#include "Editor/ImGui/misc/cpp/imgui_stdlib.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include "Serialisation/serialisation.h"

#include <fstream>
#include <json/json.hpp>
#include <iostream>

using json = nlohmann::ordered_json;

GameConfigUI::GameConfigUI(Editor& editor) :
    editor		{ editor },
	gameConfig	{ editor.engine.gameConfig }
{}

GameConfigUI::~GameConfigUI() {
	Serialiser::serialiseGameConfig("gameConfig.json", gameConfig);
}

void GameConfigUI::update() {
  
    ImGui::Begin("Game Configuration");

    if (ImGui::CollapsingHeader("Window Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // using the member variable directly
        ImGui::Text("Window Name");
		ImGui::InputText("##WindowName", &gameConfig.gameName);
        
        ImGui::Text("Game Width");
        ImGui::InputInt("##GameWidth", &gameConfig.gameWidth);
        if (gameConfig.gameWidth < 1) gameConfig.gameWidth = 1;

        ImGui::Text("Game Height");
        ImGui::InputInt("##GameHeight", &gameConfig.gameHeight);
        if (gameConfig.gameHeight < 1) gameConfig.gameHeight = 1;
    
		ImGui::Text("Start Up Scene");
		editor.displayAssetDropDownList<Scene>(gameConfig.sceneStartUp, "##startUp", [&](ResourceID scene) {
			gameConfig.sceneStartUp = scene;
		});

		float gravity = gameConfig.gravityStrength;

		ImGui::Text("Gravity");
		ImGui::DragFloat("##Gravity", &gravity);
	
		if (gravity != gameConfig.gravityStrength) {
			gameConfig.gravityStrength = gravity;
			editor.engine.physicsManager.setGravity(gravity);
		}
	}

    ImGui::Separator();

    // Action buttons
    if (ImGui::Button("Save")) {
		Serialiser::serialiseGameConfig("gameConfig.json", gameConfig);
    }

    ImGui::End();
}
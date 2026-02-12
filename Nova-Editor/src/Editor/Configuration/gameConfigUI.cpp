#include "gameConfigUI.h"
#include "imgui.h"
#include "Editor/ImGui/misc/cpp/imgui_stdlib.h"
#include "Editor/editor.h"
#include "Engine/engine.h"
#include "Serialisation/serialisation.h"
#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

#include <fstream>
#include <json/json.hpp>
#include <iostream>

using json = nlohmann::ordered_json;

GameConfigUI::GameConfigUI(Editor& editor) :
    editor		{ editor },
	gameConfig	{ editor.engine.gameConfig }
{}

GameConfigUI::~GameConfigUI() {
	Serialiser::serialiseConfig<GameConfig>("gameConfig.json", gameConfig);
}

void GameConfigUI::update() {
  
    ImGui::Begin("Game Configuration");

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

	ImGui::Text("Diffuse Environment Map");

	editor.displayAssetDropDownList<CubeMap, true>(gameConfig.environmentDiffuseMap, "##diffuseMap", [&](ResourceID scene) {
		gameConfig.environmentDiffuseMap = TypedResourceID<CubeMap>{ scene };
	});

	ImGui::Text("Diffuse Specular Map");

	editor.displayAssetDropDownList<CubeMap, true>(gameConfig.environmentSpecularMap, "##specularMap", [&](ResourceID scene) {
		gameConfig.environmentSpecularMap = TypedResourceID<CubeMap>{ scene };
	});

	float gravity = gameConfig.gravityStrength;

	ImGui::Text("Gravity");
	ImGui::DragFloat("##Gravity", &gravity);
	
	if (gravity != gameConfig.gravityStrength) {
		gameConfig.gravityStrength = gravity;
		editor.engine.physicsManager.setGravity(gravity);
	}

	DisplayProperty<NormalizedFloat>(editor, "IBL Diffuse Strength", gameConfig.iblDiffuseStrength);
	DisplayProperty<NormalizedFloat>(editor, "IBL Specular Strength", gameConfig.iblSpecularStrength);

    ImGui::Separator();

    // Action buttons
    if (ImGui::Button("Save")) {
		Serialiser::serialiseConfig<GameConfig>("gameConfig.json", gameConfig);
    }

    ImGui::End();
}
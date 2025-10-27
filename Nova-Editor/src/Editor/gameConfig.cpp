#include "Engine/engine.h"

#include "gameConfig.h"
#include "editor.h"
#include "imgui.h"

#include <string>

GameConfig::GameConfig(Editor& editor) :
	editor{ editor },
	engine{ editor.engine },
	gameWidth{ engine.getGameWidth() },
	gameHeight{ engine.getGameHeight() },
	name{"Nova Game"}
{
}

void GameConfig::update() {
	ImGui::Begin("GameConfig");

	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Window Name:");
	ImGui::SameLine();
	ImGui::InputText("##Window Name:", name, sizeof(name));

	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Game Height:");
	ImGui::SameLine();
	ImGui::InputInt("##Game Height", &gameHeight);


	ImGui::Dummy(ImVec2(10.0f, 0.0f));
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Game Width:");
	ImGui::SameLine();
	ImGui::InputInt("##Game Width", &gameWidth);

	engine.setGameHeight(gameHeight);
	engine.setGameWidth(gameWidth);

	ImGui::End();
}

int GameConfig::getGameWidth() {
	return gameWidth;
}

int GameConfig::getGameHeight() {
	return gameHeight;
}

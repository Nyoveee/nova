#include "gameConfigUI.h"
#include "imgui.h"

GameConfigUI::GameConfigUI(Editor& editor) :
	editor	{ editor }
{}

void GameConfigUI::update() {
	ImGui::Begin("Game Configuration.");

	ImGui::End();
}
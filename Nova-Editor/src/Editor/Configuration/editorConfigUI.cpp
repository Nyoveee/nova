#include "editorConfigUI.h"
#include "imgui.h"

EditorConfigUI::EditorConfigUI(Editor& editor) :
	editor { editor }
{}

void EditorConfigUI::update() {
	ImGui::Begin("Editor Configuration.");

	ImGui::End();
}
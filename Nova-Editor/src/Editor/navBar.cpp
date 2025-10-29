#include "Engine/engine.h"

#include "navBar.h"
#include "editor.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/window.h"
#include "Graphics/renderer.h"


#include <format>

NavBar::NavBar(Editor& editor) :
	engine{editor.engine},
	debugUi{editor},
	hierarchyList{editor},
	componentInspector{editor},
	gameConfig{editor},
	animationWindow{editor.animationTimeLine},
	animatorWindow{editor.animatorController},
	navigationWindow{editor.navigationWindow},
	consoleBool{true},
	debugUiBool{true},
	hierarchyBool{true},
	componentInspectorBool{true},
	gameConfigBool{true},
	animationBool{true},
	animatorBool{true},
	imGuiDemoBool{true},
	navigationBool{true}
{}

void NavBar::update() {

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::MenuItem("console", nullptr ,&consoleBool)) {	}
			if (ImGui::MenuItem("debugUi", nullptr ,&debugUiBool))	{	}
			if (ImGui::MenuItem("hierarchy", nullptr, &hierarchyBool)) {		}
			if (ImGui::MenuItem("componentInspector", nullptr, &componentInspectorBool)) {	}
			if (ImGui::MenuItem("GameConfig", nullptr, &gameConfigBool)) {	}
			if (ImGui::MenuItem("Animation", nullptr, &animationBool)) {}
			if (ImGui::MenuItem("Animator", nullptr, &animatorBool)) {}
			if (ImGui::MenuItem("ImGui Demo", nullptr, &imGuiDemoBool)) {}
			if (ImGui::MenuItem("Navigation", nullptr, &navigationBool)) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	if (consoleBool) {
		console.update();
	}
	if (debugUiBool) {
		debugUi.update();
	}
	if (hierarchyBool) {
		hierarchyList.update();
	}
	if (componentInspectorBool) {
		componentInspector.update();
	}
	if (gameConfigBool) {
		gameConfig.update();
	}
	if (animationBool) {
		animationWindow.update();
	}
	if (animatorBool) {
		animatorWindow.update();
	}
	if (imGuiDemoBool) {
		ImGui::ShowDemoWindow();
	}
	if (navigationBool) {
		navigationWindow.update();
	}
}
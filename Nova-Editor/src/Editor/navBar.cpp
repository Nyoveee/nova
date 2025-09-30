#include "Engine/engine.h"

#include "navBar.h"
#include "editor.h"
#include "ResourceManager/resourceManager.h"
#include "Engine/window.h"
#include "Graphics/renderer.h"


#include <format>

NavBar::NavBar(Editor& editor) :
	engine{ editor.engine },
	debugUi{ editor },
	hierarchyList{editor},
	componentInspector{editor},
	gameConfig{editor},
	consoleBool{true},
	debugUiBool{true},
	hierarchyBool{true},
	componentInspectorBool{true},
	gameConfigBool{true}
{}

void NavBar::update() {

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::Checkbox("console",&consoleBool)) {	}
			if (ImGui::Checkbox("debugUi", &debugUiBool))	{	}
			if (ImGui::Checkbox("hierarchy", &hierarchyBool)) {		}
			if (ImGui::Checkbox("componentInspector", &componentInspectorBool)) {	}
			if (ImGui::Checkbox("GameConfig", &gameConfigBool)) {	}
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
	
}
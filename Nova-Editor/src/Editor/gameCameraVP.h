#pragma once

#include <glad/glad.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "gizmo.h"
#include "controlOverlay.h"

class Engine;
class Editor;

class GameCamViewPort {
public:
	GameCamViewPort(Editor& editor);
	void update(float dt);

public:
	bool isHoveringOver;
	//ControlOverlay controlOverlay;

private:
	Editor& editor;
	Engine& engine;
	//Gizmo gizmo;
};
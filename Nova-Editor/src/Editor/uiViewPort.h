#pragma once

#include <glad/glad.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "gizmo.h"
#include "controlOverlay.h"

class Engine;
class Editor;

class UIViewPort {
public:
	UIViewPort(Editor& editor);

public:
	void update();

public:
	bool isHoveringOver;
	bool isActive;
	ControlOverlay controlOverlay;

private:
	Editor& editor;
	Engine& engine;
	Gizmo gizmo;
};
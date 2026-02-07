#pragma once

#include <glad/glad.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "gizmo.h"
#include "controlOverlay.h"

class Engine;
class Editor;

class EditorViewPort {
public:
	EditorViewPort(Editor& editor);

public:
	void update(float dt);

public:
	bool isHoveringOver;
	bool isActive;
	ControlOverlay controlOverlay;

private:
	Editor& editor;
	Engine& engine;

public:
	Gizmo gizmo;
};
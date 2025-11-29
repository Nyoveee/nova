#pragma once

#include <glad/glad.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "gizmo.h"
#include "controlOverlay.h"

class Engine;
class Editor;

class GameViewPort {
public:
	GameViewPort(Editor& editor);

public:
	void update(float dt);
private:
	Editor& editor;
	Engine& engine;
};
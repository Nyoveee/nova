#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

class Engine;

class GameViewPort {
public:
	GameViewPort(Engine& engine);
	void update();

public:
	bool isHoveringOver;

	// value is normalized, but may range outside of [0, 1] (because mouse is outside the viewport).
	// also, positive y points upwards.
	ImVec2 mouseRelativeToViewPort;	

private:
	Engine& engine;
};
#pragma once

#include "imgui.h"
#include "ImGuizmo.h"

class Editor;
class ECS;

class Gizmo {
public:
	Gizmo(Editor& editor, ECS& ecs);

public:
	void update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight, 
				float const* view, float const* proj);

private:
	ECS& ecs;
	Editor& editor;

	ImGuizmo::OPERATION operation;
};
#pragma once

#include "imgui.h"
#include "ImGuizmo.h"

class Editor;
class ECS;

class Gizmo {
public:
	Gizmo(Editor& editor, ECS& ecs);

public:
	void update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight, bool isUI = false);

public:
	ImGuizmo::MODE mode = ImGuizmo::WORLD;

	float translationSnappingValue = 1.f;
	float scaleSnappingValue = 1.f;
	float rotationSnappingValue = 45.f;

	bool isSnapping = false;

private:
	ECS& ecs;
	Editor& editor;

	ImGuizmo::OPERATION operation;

};
#pragma once

#include "imgui.h"
#include "ImGuizmo.h"

class Editor;
class ECS;

class Gizmo {
public:
	Gizmo(Editor& editor, ECS& ecs);

public:
	void update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight, entt::registry& registry);

private:
	ECS& ecs;
	Editor& editor;

	ImGuizmo::OPERATION operation;
};
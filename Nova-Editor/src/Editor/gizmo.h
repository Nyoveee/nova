#pragma once

#include "imgui.h"
#include "ImGuizmo.h"
#include "nova_math.h"

class Editor;
class ECS;

enum class EditingMode {
	Transform,
	NavMeshStart,
	NavMeshEnd
};

class Gizmo {
public:
	Gizmo(Editor& editor, ECS& ecs);

public:
	void update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight, bool isUI = false);

private:
	Math::DecomposedMatrix showGizmo(glm::mat4& modelMatrix, bool isUI);

public:
	ImGuizmo::MODE mode = ImGuizmo::WORLD;

	float translationSnappingValue = 1.f;
	float scaleSnappingValue = 1.f;
	float rotationSnappingValue = 45.f;

	bool isSnapping = false;

	EditingMode editingMode = EditingMode::Transform;

private:
	ECS& ecs;
	Editor& editor;

	ImGuizmo::OPERATION operation;

};
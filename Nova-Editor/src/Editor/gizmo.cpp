#include "Engine/engine.h"

#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "ImGuizmo.h"

#include "gizmo.h"
#include "editor.h"
#include "ECS/ecs.h"
#include "InputManager/inputManager.h"
#include "InputManager/inputEvent.h"
#include "component.h"
#include "nova_math.h"

Gizmo::Gizmo(Editor& editor, ECS& ecs) : 
	editor		{ editor },
	ecs			{ ecs },
	operation	{ ImGuizmo::OPERATION::TRANSLATE }
{
	editor.inputManager.subscribe<GizmoMode>(
		[&](GizmoMode gizmoMode) {
			if (!editor.isActive() || ImGui::IsAnyItemActive()) {
				return;
			}

			switch (gizmoMode)
			{
			case GizmoMode::Scale:
				operation = ImGuizmo::OPERATION::SCALE;
				break;
			case GizmoMode::Rotate:
				operation = ImGuizmo::OPERATION::ROTATE;
				break;
			case GizmoMode::Translate:
				operation = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
		}
	);
}

void Gizmo::update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight) {
	if (!editor.hasAnyEntitySelected()) {
		return;
	}

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(viewportPosX, viewportPosY, viewportWidth, viewportHeight);

	entt::entity selectedEntity = editor.getSelectedEntities()[0];

	Transform& transform = ecs.registry.get<Transform>(selectedEntity);
	float const* cameraView = glm::value_ptr(editor.engine.renderer.getCamera().view());
	float const* cameraProjection = glm::value_ptr(editor.engine.renderer.getCamera().projection());

	ImGuizmo::Manipulate(cameraView, cameraProjection, operation, ImGuizmo::WORLD, glm::value_ptr(transform.modelMatrix));

	if (!ImGuizmo::IsUsing()) {
		return;
	}

	auto [position, rotation, scale] = Math::decomposeMatrix(transform.modelMatrix);

	transform.position = position;
	transform.scale = scale;

	if (!glm::all(glm::epsilonEqual(transform.rotation, rotation, 1e-4f))) {
		transform.rotation = rotation;
	}
}
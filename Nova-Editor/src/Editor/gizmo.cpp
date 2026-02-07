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

void Gizmo::update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight, bool isUI) {
	if (!editor.hasAnyEntitySelected()) {
		return;
	}

	// Early exit depending on whether selected entities has ui components
	const std::vector<entt::entity>& selectedEntities = editor.getSelectedEntities();

	for (entt::entity entity : selectedEntities) {
		if (ecs.isParentCanvas(entity)) {
			if (!isUI) {
				return;
			}
		}
		else if (isUI) {
			return;
		}
	}

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(viewportPosX, viewportPosY, viewportWidth, viewportHeight);

	entt::entity selectedEntity = editor.getSelectedEntities()[0];

	switch (editingMode) {
	case EditingMode::Transform: {
		Transform& transform = ecs.registry.get<Transform>(selectedEntity);

		auto [position, rotation, scale] = showGizmo(transform.modelMatrix, isUI);

		if (!ImGuizmo::IsUsing()) {
			return;
		}

		transform.position = position;
		transform.scale = scale;

		if (!glm::all(glm::epsilonEqual(transform.rotation, rotation, 1e-4f))) {
			transform.rotation = rotation;
		}
		break;
	}
	case EditingMode::NavMeshStart: {
		NavMeshOffLinks* navMeshLink = ecs.registry.try_get<NavMeshOffLinks>(selectedEntity);

		if (!navMeshLink) {
			break;
		}

		// show start point gizmo
		glm::mat4 startModelMatrix = Math::composeMatrix(navMeshLink->startPoint, glm::identity<glm::quat>(), glm::vec3{ 1.f });
		auto [startPointPosition, startPointrotation, startPointScale] = showGizmo(startModelMatrix, isUI);

		if (!ImGuizmo::IsUsing()) {
			break;
		}

		navMeshLink->startPoint = startPointPosition;
		break;
	}

	case EditingMode::NavMeshEnd: {
		NavMeshOffLinks* navMeshLink = ecs.registry.try_get<NavMeshOffLinks>(selectedEntity);

		if (!navMeshLink) {
			break;
		}

		// show end point gizmo
		glm::mat4 endModelMatrix = Math::composeMatrix(navMeshLink->endPoint, glm::identity<glm::quat>(), glm::vec3{ 1.f });
		auto [endPointPosition, endPointrotation, endPointScale] = showGizmo(endModelMatrix, isUI);

		if (!ImGuizmo::IsUsing()) {
			break;
		}

		navMeshLink->endPoint = endPointPosition;
		break;
	}
	}
}

Math::DecomposedMatrix Gizmo::showGizmo(glm::mat4& modelMatrix, bool isUI) {

	ImGuizmo::SetOrthographic(isUI);
	glm::mat4 viewMat = isUI ? glm::mat4(1.0f) : editor.engine.renderer.getEditorCamera().view();
	glm::mat4 projMat = isUI ? glm::mat4(1.0f) : editor.engine.renderer.getEditorCamera().projection();

	// variable here because of lifetime.
	glm::vec3 snappingValues;

	const float* snapping = [&]() -> const float* {
		if (!isSnapping) {
			return nullptr;
		}

		switch (operation) {
			using enum ImGuizmo::OPERATION;
		case SCALE:
			snappingValues = { scaleSnappingValue, scaleSnappingValue , scaleSnappingValue };
			break;
		case ROTATE:
			snappingValues = { rotationSnappingValue, rotationSnappingValue , rotationSnappingValue };
			break;
		case TRANSLATE:
			snappingValues = { translationSnappingValue, translationSnappingValue , translationSnappingValue };
			break;
		default:
			return nullptr;
		}

		return glm::value_ptr(snappingValues);
	}();

	ImGuizmo::Manipulate(
		glm::value_ptr(viewMat),
		glm::value_ptr(projMat),
		operation,
		mode,
		glm::value_ptr(modelMatrix),
		nullptr,
		snapping
	);

	return Math::decomposeMatrix(modelMatrix);
}

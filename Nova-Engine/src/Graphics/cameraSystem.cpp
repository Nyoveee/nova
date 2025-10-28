#include <cmath>

#include "Engine/engine.h"
#include "cameraSystem.h"
#include "renderer.h"
#include "InputManager/inputManager.h"
#include "Profiling.h"

CameraSystem::CameraSystem(Engine& engine) :
	engine					{ engine },
	isMovingDown			{},
	isMovingUp				{},
	isMovingFront			{},
	isMovingBack			{},
	isMovingLeft			{},
	isMovingRight			{},
	isSimulationActive		{},
	lastMouseX				{},
	lastMouseY				{},
	editorCamera			{ engine.renderer.getEditorCamera() },
	gameCamera				{ engine.renderer.getGameCamera() },
	levelEditorCamera		{ { 0.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, -90.f, 0.f },
	isInControl				{ false },
	isThereActiveGameCamera	{ false },
	toResetMousePos			{ true },
	cameraSpeedExponent		{ 2.f },
	cameraSpeed				{ std::exp(cameraSpeedExponent) },
	focusOffsetDistance		{ 5.0f },
	focusHeightOffset		{ 2.0f }
{
	// Subscribe to the input manager that the camera system is interested in 
	// any input related to CameraMovement
	engine.inputManager.subscribe<CameraMovement>(
		[&](CameraMovement movement) {
			setMovement(movement, true);
		},
		[&](CameraMovement movement) {
			setMovement(movement, false);
		}
	);

	engine.inputManager.subscribe<ToCameraControl>(
		[&](ToCameraControl control) {
			if (isSimulationActive && isThereActiveGameCamera) {
				return;
			}

			isInControl = control == ToCameraControl::Control;

			if (isInControl) {
				toResetMousePos = true;
			}
		}
	);

	engine.inputManager.subscribe<MousePosition>(
		[&](MousePosition mousePos) {
			if (toResetMousePos) {
				setLastMouse(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
				toResetMousePos = false;
			}
			else if (isInControl) {
				calculateEulerAngle(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
			}
		}
	);

	engine.inputManager.subscribe<AdjustCameraSpeed>(
		[&](AdjustCameraSpeed value) {
			if (!isInControl) {
				return;
			}

			constexpr float adjustmentMultiplier = 0.1f;
			cameraSpeedExponent += static_cast<float>(value.value) * adjustmentMultiplier;
		}
	);
}

void CameraSystem::update(float dt) {
	ZoneScoped;

	for (auto&& [entityID, cameraComponent] : engine.ecs.registry.view<CameraComponent>().each())
	{
		if (cameraComponent.camStatus)
		{
			Transform& objTransform = engine.ecs.registry.get<Transform>(entityID);
			// Use Transform data to set camera variables.
			gameCamera.setPos(objTransform.position);
			gameCamera.setFront(objTransform.front);

			gameCamera.recalculateViewMatrix();
			gameCamera.recalculateProjectionMatrix();
			break;
		}
	}

	// for editor camera
	if(!isSimulationActive || !isThereActiveGameCamera)
	{
		if (!isInControl) {
			return;
		}

		cameraSpeed = std::exp(cameraSpeedExponent);

		if (isMovingFront) {
			levelEditorCamera.position += cameraSpeed * dt * editorCamera.getFront();
		}

		if (isMovingLeft) {
			levelEditorCamera.position -= cameraSpeed * dt * editorCamera.getRight();
		}

		if (isMovingBack) {
			levelEditorCamera.position -= cameraSpeed * dt * editorCamera.getFront();
		}

		if (isMovingRight) {
			levelEditorCamera.position += cameraSpeed * dt * editorCamera.getRight();
		}

		if (isMovingUp) {
			levelEditorCamera.position += cameraSpeed * dt * Camera::Up;
		}

		if (isMovingDown) {
			levelEditorCamera.position -= cameraSpeed * dt * Camera::Up;
		}
	
		editorCamera.setPos(levelEditorCamera.position);
		editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	}
	
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();

}

void CameraSystem::setMovement(CameraMovement movement, bool toMove) {
	switch (movement)
	{
	case CameraMovement::Forward:
		isMovingFront = toMove;
		break;
	case CameraMovement::Backward:
		isMovingBack = toMove;
		break;
	case CameraMovement::Left:
		isMovingLeft = toMove;
		break;
	case CameraMovement::Right:
		isMovingRight = toMove;
		break;
	case CameraMovement::Ascend:
		isMovingUp = toMove;
		break;
	case CameraMovement::Descent:
		isMovingDown = toMove;
		break;
	default:
		break;
	}
}

void CameraSystem::setLastMouse(float mouseX, float mouseY) {
	lastMouseX = mouseX;
	lastMouseY = mouseY;
}

float CameraSystem::getCameraSpeed() const {
	return cameraSpeed;
}

CameraSystem::LevelEditorCamera const& CameraSystem::getLevelEditorCamera() const {
    return levelEditorCamera;
}

void CameraSystem::calculateEulerAngle(float mouseX, float mouseY) {
	constexpr float sensitivity = 0.1f;		// change this value to your liking
	
	float xOffset = mouseX - lastMouseX;
	float yOffset = lastMouseY - mouseY; // reversed since y-coordinates go from bottom to top
	lastMouseX = mouseX;
	lastMouseY = mouseY;

	xOffset *= sensitivity;
	yOffset *= sensitivity;

	levelEditorCamera.yaw += xOffset;
	levelEditorCamera.pitch += yOffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	levelEditorCamera.pitch = std::clamp(static_cast<float>(levelEditorCamera.pitch), -89.0f, 89.0f);

	levelEditorCamera.front.x = cos(Radian{ levelEditorCamera.yaw }) * cos(Radian{ levelEditorCamera.pitch });
	levelEditorCamera.front.y = sin(Radian{ levelEditorCamera.pitch });
	levelEditorCamera.front.z = sin(Radian{ levelEditorCamera.yaw }) * cos(Radian{ levelEditorCamera.pitch });
}


void CameraSystem::startSimulation() {
	isSimulationActive = true;
}

void CameraSystem::endSimulation() {
	isSimulationActive = false;

	editorCamera.setPos(levelEditorCamera.position);
	editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();
}

void CameraSystem::focusOnPosition(glm::vec3 const& targetPosition) {
	// Position camera behind and above the target using configurable offsets
	glm::vec3 offset = glm::vec3(0.0f, focusHeightOffset, focusOffsetDistance);
	levelEditorCamera.position = targetPosition + offset;

	// Calculate direction from camera to target
	glm::vec3 direction = glm::normalize(targetPosition - levelEditorCamera.position);
	levelEditorCamera.front = direction;

	// Calculate yaw and pitch from direction vector
	// yaw: angle around y-axis (horizontal rotation)
	levelEditorCamera.yaw = Degree{ glm::degrees(std::atan2(direction.z, direction.x)) };

	// pitch: angle up/down
	levelEditorCamera.pitch = Degree{ glm::degrees(std::asin(direction.y)) };

	// Update the editor camera immediately
	editorCamera.setPos(levelEditorCamera.position);
	editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();
}
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
	camera					{ engine.renderer.getCamera() },
	levelEditorCamera		{ { 0.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, -90.f, 0.f },
	isInControl				{ false },
	toResetMousePos			{ true },
	cameraSpeedExponent		{ 2.f },
	cameraSpeed				{ std::exp(cameraSpeedExponent) }
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
			if (isSimulationActive) {
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
			else if (isInControl && !isSimulationActive) {
				calculateEulerAngle(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
			}
		}
	);

	engine.inputManager.subscribe<AdjustCameraSpeed>(
		[&](AdjustCameraSpeed value) {
			if (!isInControl || isSimulationActive) {
				return;
			}

			constexpr float adjustmentMultiplier = 0.1f;
			cameraSpeedExponent += static_cast<float>(value.value) * adjustmentMultiplier;
		}
	);
}

void CameraSystem::update(float dt) {
	ZoneScoped;
	
	// for game camera
	if(isSimulationActive)
	{
		for (auto&& [entityID, cameraComponent] : engine.ecs.registry.view<CameraComponent>().each())
		{
			if (cameraComponent.camStatus)
			{
				Transform& objTransform = engine.ecs.registry.get<Transform>(entityID);
				// Use Transform data to set camera variables.
				camera.setPos(objTransform.position);
				camera.setFront(objTransform.front);
				break;
			}
		}
	}
	// for editor camera
	else
	{
		if (!isInControl) {
			return;
		}

		cameraSpeed = std::exp(cameraSpeedExponent);

		if (isMovingFront) {
			levelEditorCamera.position += cameraSpeed * dt * camera.getFront();
		}

		if (isMovingLeft) {
			levelEditorCamera.position -= cameraSpeed * dt * camera.getRight();
		}

		if (isMovingBack) {
			levelEditorCamera.position -= cameraSpeed * dt * camera.getFront();
		}

		if (isMovingRight) {
			levelEditorCamera.position += cameraSpeed * dt * camera.getRight();
		}

		if (isMovingUp) {
			levelEditorCamera.position += cameraSpeed * dt * Camera::Up;
		}

		if (isMovingDown) {
			levelEditorCamera.position -= cameraSpeed * dt * Camera::Up;
		}
	
		camera.setPos(levelEditorCamera.position);
		camera.setFront(glm::normalize(levelEditorCamera.front));
	}
	
	camera.recalculateViewMatrix();
	camera.recalculateProjectionMatrix();
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

	camera.setPos(levelEditorCamera.position);
	camera.setFront(glm::normalize(levelEditorCamera.front));
	camera.recalculateViewMatrix();
	camera.recalculateProjectionMatrix();
}
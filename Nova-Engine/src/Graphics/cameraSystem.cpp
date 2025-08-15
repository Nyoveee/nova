#include <cmath>

#include "engine.h"
#include "cameraSystem.h"
#include "renderer.h"
#include "inputManager.h"

CameraSystem::CameraSystem(Engine& engine, InputManager& inputManager) :
	engine				{ engine },
	isMovingDown		{},
	isMovingUp			{},
	isMovingFront		{},
	isMovingBack		{},
	isMovingLeft		{},
	isMovingRight		{},
	lastMouseX			{},
	lastMouseY			{},
	camera				{ engine.renderer.getCamera() },
	yaw					{ -90.f },
	pitch				{},
	isInControl			{ false },
	toResetMousePos		{ true },
	cameraSpeedExponent { 2.f },
	cameraSpeed			{ std::exp(cameraSpeedExponent) }
{
	// Subscribe to the input manager that the camera system is interested in 
	// any input related to CameraMovement
	inputManager.subscribe<CameraMovement>(
		[&](CameraMovement movement) {
			setMovement(movement, true);
		},
		[&](CameraMovement movement) {
			setMovement(movement, false);
		}
	);

	inputManager.subscribe<ToCameraControl>(
		[&](ToCameraControl control) {
			isInControl = control == ToCameraControl::Control;

			if (isInControl) {
				toResetMousePos = true;
			}
		}
	);

	inputManager.subscribe<MousePosition>(
		[&](MousePosition mousePos) {
			if (toResetMousePos) {
				setLastMouse(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
				toResetMousePos = false;
			}
			else {
				if(isInControl) calculateEulerAngle(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
			}
		}
	);



	inputManager.subscribe<AdjustCameraSpeed>(
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
	if (!isInControl) {
		return;
	}

	cameraSpeed = std::exp(cameraSpeedExponent);

	if (isMovingFront) {
		camera.addPos(cameraSpeed * dt * camera.getFront());
	}

	if (isMovingLeft) {
		camera.addPos(cameraSpeed * dt * -camera.getRight());
	}

	if (isMovingBack) {
		camera.addPos(cameraSpeed * dt * -camera.getFront());
	}

	if (isMovingRight) {
		camera.addPos(cameraSpeed * dt * camera.getRight());
	}

	if (isMovingUp) {
		camera.addPos(cameraSpeed * dt * Camera::Up);
	}

	if (isMovingDown) {
		camera.addPos(cameraSpeed * dt * -Camera::Up);
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

	yaw += xOffset;
	pitch += yOffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(Radian{ yaw }) * cos(Radian{ pitch });
	front.y = sin(Radian{ pitch });
	front.z = sin(Radian{ yaw }) * cos(Radian{ pitch });
	
	camera.setFront(glm::normalize(front));
}

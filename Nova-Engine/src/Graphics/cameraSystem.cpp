#include "cameraSystem.h"
#include "renderer.h"
#include "engine.h"
#include "inputManager.h"

CameraSystem::CameraSystem(Engine& engine, InputManager& inputManager) :
	engine			{ engine },
	isMovingDown	{},
	isMovingUp		{},
	isMovingFront	{},
	isMovingBack	{},
	isMovingLeft	{},
	isMovingRight	{},
	lastMouseX		{},
	lastMouseY		{},
	camera			{ engine.renderer.getCamera() },
	yaw				{ -90.f },
	pitch			{}
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

	inputManager.subscribe<MousePosition>(
		[&](MousePosition mousePos) {
			static bool firstTime = true;

			if (firstTime) {
				setLastMouse(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
				firstTime = false;
			}
			else {
				calculateEulerAngle(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
			}
		}
	);
}

void CameraSystem::update() {
	constexpr float cameraSpeed = 0.05f; // adjust accordingly

	if (isMovingFront) {
		camera.addPos(cameraSpeed * camera.getFront());
	}

	if (isMovingLeft) {
		camera.addPos(cameraSpeed * -camera.getRight());
	}

	if (isMovingBack) {
		camera.addPos(cameraSpeed * -camera.getFront());
	}

	if (isMovingRight) {
		camera.addPos(cameraSpeed * camera.getRight());
	}

	if (isMovingUp) {
		camera.addPos(cameraSpeed * Camera::Up);
	}

	if (isMovingDown) {
		camera.addPos(cameraSpeed * -Camera::Up);
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

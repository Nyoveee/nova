#include "cameraSystem.h"
#include "renderer.h"

CameraSystem::CameraSystem() :
	isMovingDown	{},
	isMovingUp		{},
	isMovingFront	{},
	isMovingBack	{},
	isMovingLeft	{},
	isMovingRight	{},
	lastMouseX		{},
	lastMouseY		{},
	camera			{ Renderer::instance().getCamera() },
	yaw				{ -90.f },
	pitch			{}
{}

CameraSystem& CameraSystem::instance() {
	static CameraSystem cameraSystem{};
	return cameraSystem;
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

void CameraSystem::setMovement(Movement movement, bool toMove) {
	switch (movement)
	{
	case CameraSystem::Movement::Up:
		isMovingUp = toMove;
		break;
	case CameraSystem::Movement::Down:
		isMovingDown = toMove;
		break;
	case CameraSystem::Movement::Front:
		isMovingFront = toMove;
		break;
	case CameraSystem::Movement::Back:
		isMovingBack = toMove;
		break;
	case CameraSystem::Movement::Left:
		isMovingLeft = toMove;
		break;
	case CameraSystem::Movement::Right:
		isMovingRight = toMove;
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

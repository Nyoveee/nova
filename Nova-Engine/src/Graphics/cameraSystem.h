#pragma once

#include "Libraries/type_alias.h"
#include "InputManager/inputEvent.h"

class Camera;
class Engine;
class InputManager;

class CameraSystem {
public:
	CameraSystem(Engine& engine, InputManager& inputManager);

	~CameraSystem()										= default;
	CameraSystem(CameraSystem const& other)				= delete;
	CameraSystem(CameraSystem&& other)					= delete;
	CameraSystem& operator=(CameraSystem const& other)	= delete;
	CameraSystem& operator=(CameraSystem&& other)		= delete;
	
public:
	void update();

	void setMovement(CameraMovement movement, bool toMove);

	void setLastMouse(float mouseX, float mouseY);
	void calculateEulerAngle(float mouseX, float mouseY);

private:
	Engine& engine;
	Camera& camera;

	bool isMovingUp;
	bool isMovingDown;
	bool isMovingFront;
	bool isMovingBack;
	bool isMovingLeft;
	bool isMovingRight;

	float lastMouseX;
	float lastMouseY;

	Degree yaw;
	Degree pitch;
};
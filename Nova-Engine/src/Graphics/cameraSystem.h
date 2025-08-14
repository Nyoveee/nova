#pragma once

#include "export.h"
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
	void update(float dt);

	void setMovement(CameraMovement movement, bool toMove);
	void setLastMouse(float mouseX, float mouseY);
	void calculateEulerAngle(float mouseX, float mouseY);

public:
	// the formula of camera speed is e^x, to appropriately scale speed. 
	float cameraSpeedExponent;
	
public:
	DLL_API float getCameraSpeed() const;

private:
	float cameraSpeed;

	bool isInControl;
	bool toResetMousePos;

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
#pragma once

#include "../Libraries/type_alias.h"

class Camera;

class CameraSystem {
	CameraSystem();

public:
	enum class Movement {
		Up,
		Down,
		Front,
		Back,
		Left,
		Right
	};

public:
	static CameraSystem& instance();

	~CameraSystem()										= default;
	CameraSystem(CameraSystem const& other)				= delete;
	CameraSystem(CameraSystem&& other)					= delete;
	CameraSystem& operator=(CameraSystem const& other)	= delete;
	CameraSystem& operator=(CameraSystem&& other)		= delete;
	
public:
	void update();

	void setMovement(Movement movement, bool toMove);

	void setLastMouse(float mouseX, float mouseY);
	void calculateEulerAngle(float mouseX, float mouseY);

private:
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
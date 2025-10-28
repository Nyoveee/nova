#pragma once

#include <glm/vec3.hpp>
#include "export.h"
#include "type_alias.h"
#include "InputManager/inputEvent.h"

class Camera;
class Engine;
class InputManager;

class CameraSystem {
public:
	struct LevelEditorCamera {
		glm::vec3 position;
		glm::vec3 front;

		Degree yaw;
		Degree pitch;
	};

public:
	CameraSystem(Engine& engine);

	~CameraSystem()										= default;
	CameraSystem(CameraSystem const& other)				= delete;
	CameraSystem(CameraSystem&& other)					= delete;
	CameraSystem& operator=(CameraSystem const& other)	= delete;
	CameraSystem& operator=(CameraSystem&& other)		= delete;
	
public:
	void update(float dt);

	void setMovement(CameraMovement movement, bool toMove);
	void setLastMouse(float mouseX, float mouseY);
	void calculateMouseOffset(float mouseX, float mouseY);
	void calculateEulerAngle();

	void startSimulation();
	void endSimulation();

	ENGINE_DLL_API float getCameraSpeed() const;
	ENGINE_DLL_API LevelEditorCamera const& getLevelEditorCamera() const;

	ENGINE_DLL_API float getMouseAxisX();
	ENGINE_DLL_API float getMouseAxisY();

public:
	// the formula of camera speed is e^x, to appropriately scale speed. 
	float cameraSpeedExponent;

private:
	LevelEditorCamera levelEditorCamera;

	float cameraSpeed;
	bool isSimulationActive;
	bool isThereActiveGameCamera;

	bool isInControl;
	bool toResetMousePos;

	Engine& engine;
	Camera& editorCamera;
	Camera& gameCamera;

	bool isMovingUp;
	bool isMovingDown;
	bool isMovingFront;
	bool isMovingBack;
	bool isMovingLeft;
	bool isMovingRight;

	float lastMouseX;
	float lastMouseY;

	float xOffset;
	float yOffset;
};
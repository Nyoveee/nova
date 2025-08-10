#include <GLFW/glfw3.h>

#include "inputManager.h"
#include "window.h"

#include "InputManager/inputEvent.h"

InputManager::InputManager() {}

void InputManager::handleMouseMovement(Window& window, double xPosIn, double yPosIn) {
	(void) window;

	broadcast(MousePosition{ xPosIn, yPosIn }, InputType::Press);
}

void InputManager::handleScroll(Window& window, double xOffset, double yOffset) {
	(void) window;
	(void) xOffset;

	broadcast(AdjustCameraSpeed{ yOffset });
}

void InputManager::handleKeyboardInput(Window& window, int key, int scancode, int action, int mods) {
	(void) window;
	(void) mods;
	(void) scancode;

	if (key == GLFW_KEY_W) {
		broadcast(CameraMovement::Forward, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}

	if (key == GLFW_KEY_S) {
		broadcast(CameraMovement::Backward, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}

	if (key == GLFW_KEY_A) {
		broadcast(CameraMovement::Left, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}

	if (key == GLFW_KEY_D) {
		broadcast(CameraMovement::Right, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}

	if (key == GLFW_KEY_SPACE) {
		broadcast(CameraMovement::Ascend, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}

	if (key == GLFW_KEY_Z) {
		broadcast(CameraMovement::Descent, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}
}

void InputManager::handleMouseInput(Window& window, int key, int action, int mods) {
	(void) window;
	(void) mods;

	if (key == GLFW_MOUSE_BUTTON_RIGHT) {
		broadcast(ToggleEditorControl::Sentinel, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
	}
}

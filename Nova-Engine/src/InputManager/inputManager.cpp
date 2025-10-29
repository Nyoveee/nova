#include <GLFW/glfw3.h>

#include "InputManager/inputManager.h"
#include "Engine/window.h"

#include "InputManager/inputEvent.h"

InputManager::InputManager() :
	mousePosition		{},
	lastMousePosition	{},
	currentObserverId	{ 0 }
{
	mainKeyBindMapping();
}

void InputManager::handleMouseMovement(Window& window, double xPosIn, double yPosIn) {
	(void) window;

	lastMousePosition = mousePosition;
	mousePosition = { xPosIn, yPosIn };

	glm::vec2 deltaMousePosition = lastMousePosition - glm::vec2{ xPosIn, yPosIn };

	broadcast(MousePosition{ -deltaMousePosition.x, deltaMousePosition.y }, InputType::Press);
}

void InputManager::handleScroll(Window& window, double xOffset, double yOffset) {
	(void) window;
	(void) xOffset;

	broadcast(AdjustCameraSpeed{ yOffset });
	broadcast(Scroll{ yOffset });
}

void InputManager::handleKeyboardInput(Window& window, int key, int scancode, int action, int mods) {
	(void) window;
	(void) scancode;

	handleKeyInput({ key, KeyType::Keyboard }, action == GLFW_RELEASE ? InputType::Release : InputType::Press, getInputMod(mods));

	// broadcast all ScriptingInputEvent regardless of registered key mapping.
	broadcast(ScriptingInputEvents{ key }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
}

void InputManager::handleMouseInput(Window& window, int key, int action, int mods) {
	(void) window;
	(void) mods;

	handleKeyInput({ key, KeyType::MouseClick }, action == GLFW_RELEASE ? InputType::Release : InputType::Press, getInputMod(mods));

	// broadcast all ScriptingInputEvent regardless of registered key mapping.
	broadcast(ScriptingInputEvents{ key }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
}

void InputManager::handleKeyInput(GLFWInput input, InputType inputType, InputMod mod) {
	for (std::unique_ptr<IKeyBind> const& keyBind : mappedKeyBinds[input]) {
		if (mod == keyBind->mod) {
			keyBind->broadcast(inputType);
		}
	}
}

InputMod InputManager::getInputMod(int mod) const {
	InputMod inputMod = InputMod::None;

	if (mod & GLFW_MOD_SHIFT) {
		inputMod = InputMod::Shift;
	}
	else if (mod & GLFW_MOD_CONTROL) {
		inputMod = InputMod::Ctrl;
	}

	return inputMod;
}

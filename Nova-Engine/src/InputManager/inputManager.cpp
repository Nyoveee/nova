#include <GLFW/glfw3.h>

#include "InputManager/inputManager.h"
#include "Engine/window.h"

#include "InputManager/inputEvent.h"

InputManager::InputManager() :
	mousePosition		{},
	currentObserverId	{ 0 }
{
	mainKeyBindMapping();
}

void InputManager::handleMouseMovement(Window& window, double xPosIn, double yPosIn) {
	(void) window;

	mousePosition = { xPosIn,yPosIn };
	broadcast(MousePosition{ xPosIn, yPosIn }, InputType::Press);
}

void InputManager::handleScroll(Window& window, double xOffset, double yOffset) {
	(void) window;
	(void) xOffset;
	scrollOffsetY = static_cast<float>(yOffset);
	broadcast(AdjustCameraSpeed{ yOffset });
}

void InputManager::handleKeyboardInput(Window& window, int key, int scancode, int action, int mods) {
	(void) window;
	(void) mods;
	(void) scancode;

	handleKeyInput({ key, KeyType::Keyboard }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);

	// broadcast all ScriptingInputEvent regardless of registered key mapping.
	broadcast(ScriptingInputEvents{ key }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
}

void InputManager::handleMouseInput(Window& window, int key, int action, int mods) {
	(void) window;
	(void) mods;

	handleKeyInput({ key, KeyType::MouseClick }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);

	// broadcast all ScriptingInputEvent regardless of registered key mapping.
	broadcast(ScriptingInputEvents{ key }, action == GLFW_RELEASE ? InputType::Release : InputType::Press);
}

void InputManager::handleKeyInput(GLFWInput input, InputType inputType) {
	for (std::unique_ptr<IKeyBind> const& keyBind : mappedKeyBinds[input]) {
		keyBind->broadcast(inputType);
	}
}
void InputManager::update()
{
	scrollOffsetY = 0;
}


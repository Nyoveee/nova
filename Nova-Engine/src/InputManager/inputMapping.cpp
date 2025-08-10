#include "inputManager.h"
#include "inputEvent.h"

#include <GLFW/glfw3.h>

// In this file, you determine the mapping of physical keys to arbitary input events for interested observers.

void InputManager::mainKeyBindMapping() {
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//					GLFW KEY					KEY TYPE				INPUT EVENT
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 	Mapping camera movement in the level editor..
	//
	mapKeyBindInput(	GLFW_KEY_W,					KeyType::Keyboard,		CameraMovement::Forward				);
	mapKeyBindInput(	GLFW_KEY_S,					KeyType::Keyboard,		CameraMovement::Backward			);
	mapKeyBindInput(	GLFW_KEY_A,					KeyType::Keyboard,		CameraMovement::Left				);
	mapKeyBindInput(	GLFW_KEY_D,					KeyType::Keyboard,		CameraMovement::Right				);
	mapKeyBindInput(	GLFW_KEY_Q,					KeyType::Keyboard,		CameraMovement::Ascend				);
	mapKeyBindInput(	GLFW_KEY_E,					KeyType::Keyboard,		CameraMovement::Descent				);
	//
	// 	Mapping editor control via right click in the level editor..
	//
	mapKeyBindInput(	GLFW_MOUSE_BUTTON_RIGHT,	KeyType::MouseClick,	ToggleEditorControl::Sentinel		);

}

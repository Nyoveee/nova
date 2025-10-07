#include "inputManager.h"
#include "inputEvent.h"

#include <GLFW/glfw3.h>

// In this file, you determine the mapping of physical keys to arbitrary input events for interested observers.

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
	// 	Mapping gizmo controls in the level editor..
	//
	mapKeyBindInput(	GLFW_KEY_Q,					KeyType::Keyboard,		GizmoMode::Translate				);
	mapKeyBindInput(	GLFW_KEY_W,					KeyType::Keyboard,		GizmoMode::Rotate					);
	mapKeyBindInput(	GLFW_KEY_E,					KeyType::Keyboard,		GizmoMode::Scale					);
	//
	// 	Mapping delete in the level editor..
	//
	mapKeyBindInput(	GLFW_KEY_DELETE,			KeyType::Keyboard,		DeleteSelectedEntity::Sentinel		);
	//
	// 	Mapping editor control via right click in the level editor..
	//
	mapKeyBindInput(	GLFW_MOUSE_BUTTON_RIGHT,	KeyType::MouseClick,	ToggleEditorControl::Sentinel		);
	mapKeyBindInput(	GLFW_MOUSE_BUTTON_LEFT,		KeyType::MouseClick,	ToSelectGameObject::Sentinel		);

	//mapKeyBindInput(GLFW_KEY_A, KeyType::Keyboard, ScriptingInputEvents::KeyA);
	//
	// 	Mapping copy and paste keys
	//
	mapKeyBindInput(	GLFW_KEY_C,					KeyType::Keyboard,		CopyEntity::Copy					);
	mapKeyBindInput(	GLFW_KEY_V,					KeyType::Keyboard,		PasteEntity::Paste					);
}

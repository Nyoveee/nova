#pragma once
#include <GLFW/glfw3.h>
// These are abstract input events that systems can subscribe to.

// Movement in the editor.
enum class CameraMovement {
	Forward,
	Backward,
	Left,
	Right,
	Ascend,
	Descent
};

// This event tells the editor that there is an attempt to select a game object.
enum class ToSelectGameObject {
	Sentinel
};

// This event is emitted to toggle control in game for editor.
// Editor shall act as the authority in editor control, coordinating with other systems..

// Invoking press callbacks notifies editor to take control in view port.
// Invoking release callbacks notifies editor to release control back to the editor UI.
enum class ToggleEditorControl {
	Sentinel
};

// This event is to notify the editor to attempt to delete the currently selected entity.
enum class DeleteSelectedEntity {
	Sentinel
};

// This input event is to notify the camera system to whether be active or inactive.
// This input event is not mapped to any keybind is manually emitted by the editor.
enum class ToCameraControl {
	Control,
	Release
};

// This input event is to notify the Window to enable / disable cursor.
// This input event is not mapped to any keybind is manually emitted by the editor.
enum class ToEnableCursor {
	Enable,
	Disable
};

// This input event is to change Gizmo mode.
enum class GizmoMode {
	Scale,
	Rotate,
	Translate
};

enum class ScriptingEvents
{
	Press,
	Release 
	// Collision, etc.
};

enum class ScriptingInputEvents
{
	KeyA = GLFW_KEY_A
};

// ======= Mouse movement and scroll specific ==========
// Moving mouse position
struct MousePosition {
	double xPos;
	double yPos;
};

// This event adjusts the editor's camera speed based on scroll.
struct AdjustCameraSpeed {
	double value;
};

enum class CopyEntity {
	Copy
};

enum class PasteEntity {
	Paste
};


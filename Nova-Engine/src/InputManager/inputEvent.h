#pragma once

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

// Moving mouse position
struct MousePosition {
	double xPos;
	double yPos;
};

// This event is emitted to toggle control in game for editor.
// Editor shall act as the authority in editor control, coordinating with other systems..

// Invoking press callbacks notifies editor to take control in view port.
// Invoking release callbacks notifies editor to release control back to the editor UI.
enum class ToggleEditorControl {
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

// This event adjusts the editor's camera speed based on scroll.
struct AdjustCameraSpeed {
	double value;
};
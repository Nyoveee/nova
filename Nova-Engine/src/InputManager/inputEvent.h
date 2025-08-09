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
enum class ToggleCursorControl { Sentinel };
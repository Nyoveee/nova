#pragma once

// These are abstract input events that systems can subscribe to.

enum class CameraMovement {
	Forward,
	Backward,
	Left,
	Right,
	Ascend,
	Descent
};

struct MousePosition {
	double xPos;
	double yPos;
};
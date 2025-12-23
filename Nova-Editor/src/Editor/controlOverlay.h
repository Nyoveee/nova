#pragma once

#include <limits>
#include <string>

#undef max

class Editor;
class Gizmo;

constexpr float FOREVER = std::numeric_limits<float>::max();

class ControlOverlay {
public:
	ControlOverlay(Editor& editor, Gizmo& gizmo);

public:
	void update(float dt, float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);

	void setNotification(std::string text, float duration);
	void clearNotification();

private:
	void displayTopControlBar(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);
	void displayNotification(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);
	void displayGridSnappingSettings();

private:
	Editor& editor;
	Gizmo& gizmo;

	std::string notificationText;
	float notificationDuration;
	float timeElapsed;
};
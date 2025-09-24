#pragma once

#include <limits>
#include <string>

#undef max

class Editor;

constexpr float FOREVER = std::numeric_limits<float>::max();

class ControlOverlay {
public:
	ControlOverlay(Editor& editor);

public:
	void update(float dt, float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);

	void setNotification(std::string text, float duration);
	void clearNotification();

private:
	void displayTopControlBar(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);
	void displayNotification(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);

private:
	Editor& editor;

	std::string notificationText;
	float notificationDuration;
	float timeElapsed;
};
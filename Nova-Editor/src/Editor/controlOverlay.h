#pragma once

class Editor;

class ControlOverlay {
public:
	ControlOverlay(Editor& editor);

public:
	void update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight);

private:
	Editor& editor;
};
#pragma once

#include "export.h"

class Window;
class Engine;

class Editor {
public:
	Editor(Window& window, Engine& engine);

	~Editor();
	Editor(Editor const& other)				= delete;
	Editor(Editor&& other)					= delete;
	Editor& operator=(Editor const& other)	= delete;
	Editor& operator=(Editor&& other)		= delete;

public:
	void update();

private:
	void main();

private:
	Window& window;
	Engine& engine;
};
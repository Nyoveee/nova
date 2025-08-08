#pragma once

#include "export.h"

class Window;

class DLL_API Editor {
public:
	Editor(Window& window);

	~Editor();
	Editor(Editor const& other)				= delete;
	Editor(Editor&& other)					= delete;
	Editor& operator=(Editor const& other)	= delete;
	Editor& operator=(Editor&& other)		= delete;

public:
	void update();

private:
	Window& window;
};
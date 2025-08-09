#pragma once

#include "export.h"

#include "gameViewPort.h"
#include "componentInspector.h"
#include "assetManagerUi.h"

using GLuint = unsigned int;

class Window;
class Engine;
class InputManager;

class Editor {
public:
	Editor(Window& window, Engine& engine, InputManager& inputManager);

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

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetManagerUI assetManagerUi;
};
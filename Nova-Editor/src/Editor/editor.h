#pragma once

#include <vector>

#include "ecs.h"
#include "export.h"
#include "gameViewPort.h"
#include "componentInspector.h"
#include "assetManagerUi.h"

using GLuint = unsigned int;

class Window;
class Engine;
class InputManager;
class AssetManager;

class Editor {
public:
	Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager);

	~Editor();
	Editor(Editor const& other)				= delete;
	Editor(Editor&& other)					= delete;
	Editor& operator=(Editor const& other)	= delete;
	Editor& operator=(Editor&& other)		= delete;

public:
	void update();

private:
	void main();
	void toggleViewPortControl(bool toControl);
	void updateMaterialMapping();
	void handleEntitySelection();

private:
	Window& window;
	Engine& engine;
	InputManager& inputManager;
	AssetManager& assetManager;

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetManagerUI assetManagerUi;

private:
	std::vector<entt::entity> selectedEntities;
	entt::entity hoveringEntity;

	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;
};
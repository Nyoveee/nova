#pragma once

#include <unordered_set>

#include "ecs.h"
#include "export.h"
#include "gameViewPort.h"
#include "componentInspector.h"
#include "assetManagerUi.h"
#include "hierarchy.h"

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

public:
	// other editor systems have access to these
	std::unordered_set<entt::entity> selectedEntities;
	entt::entity hoveringEntity;

private:
	void main();
	void toggleViewPortControl(bool toControl);
	void updateMaterialMapping();
	void handleEntitySelection();
	void sandboxWindow();

private:
	Window& window;
	Engine& engine;
	InputManager& inputManager;
	AssetManager& assetManager;

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetManagerUI assetManagerUi;
	Hierarchy hierarchyList;

private:
	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;
};
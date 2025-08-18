#pragma once

#include <vector>

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
	bool isEntitySelected(entt::entity entity);

public:
	// other editor systems have access to these
	std::vector<entt::entity> selectedEntities;
	entt::entity hoveringEntity;

private:
	void main();
	void toggleViewPortControl(bool toControl);
	void updateMaterialMapping();
	void handleEntityHovering();
	void handleEntitySelection();
	void sandboxWindow();
	
	// loses keyboard and window focus of any window.
	void loseFocus();
public:
	// so that all editors have access to engine interface.
	Engine& engine;

private:
	Window& window;
	InputManager& inputManager;
	AssetManager& assetManager;

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetManagerUI assetManagerUi;
	Hierarchy hierarchyList;

private:
	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;
	bool toLoseFocus;
};
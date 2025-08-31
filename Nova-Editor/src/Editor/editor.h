#pragma once

#include <vector>
#include <functional>

#include "ecs.h"
#include "export.h"
#include "gameViewPort.h"
#include "componentInspector.h"
#include "assetManagerUi.h"
#include "hierarchy.h"
#include "debugUI.h"

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
	void update(std::function<void(bool)> changeSimulationCallback);
	
	bool isEntitySelected(entt::entity entity);
	bool hasAnyEntitySelected() const;
	
	void selectEntities(std::vector<entt::entity> entities);
	std::vector<entt::entity> const& getSelectedEntities() const;

	bool isActive() const;
	bool isEntityValid(entt::entity entity) const;
	void deleteEntity(entt::entity entity);

	void startSimulation();
	void stopSimulation();
	bool isInSimulationMode() const;

public:
	entt::entity hoveringEntity;

private:
	void main();
	void toggleViewPortControl(bool toControl);
	void updateMaterialMapping();
	void handleEntityValidity();
	void handleEntityHovering();
	void handleEntitySelection();
	void sandboxWindow();
	void launchProfiler();

	void toOutline(std::vector<entt::entity> const& entities, bool toOutline) const;

public:
	// so that all editors have access to engine interface.
	Engine& engine;
	InputManager& inputManager;
	AssetManager& assetManager;

private:
	Window& window;

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetManagerUI assetManagerUi;
	Hierarchy hierarchyList;
	DebugUI debugUi;

private:
	std::vector<entt::entity> selectedEntities;

	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;

	// This indicates whether the editor is in simulation mode.
	bool inSimulationMode;
	bool isThereChangeInSimulationMode;
};
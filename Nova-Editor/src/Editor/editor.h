#pragma once

#include <vector>
#include <functional>

#include "export.h"

#include "ECS/ecs.h"

#include "assetViewerUi.h"
#include "gameViewPort.h"
#include "ComponentInspection/componentInspector.h"
#include "assetManagerUi.h"
#include "Navigation/navMeshGeneration.h"
#include "hierarchy.h"
#include "debugUI.h"
#include "console.h"
#include "navBar.h"
#include "navigationWindow.h"

using GLuint = unsigned int;

class Window;
class Engine;
class InputManager;
class AssetManager;
class ResourceManager;

class Editor {
public:
	Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager, ResourceManager& resourceManager);

	~Editor();
	Editor(Editor const& other)				= delete;
	Editor(Editor&& other)					= delete;
	Editor& operator=(Editor const& other)	= delete;
	Editor& operator=(Editor&& other)		= delete;

public:
	void update(float dt, std::function<void(bool)> changeSimulationCallback);
	
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
	// displays a ImGui combo drop down box of all the assets related to type T.
	// first parameter is used to specific which asset id is selected.
	template <typename T>
	void displayAssetDropDownList(std::optional<ResourceID> id, const char* labelName, std::function<void(ResourceID)> onClickCallback);
	
	void launchProfiler();

public:
	entt::entity hoveringEntity;
	std::vector<entt::entity> copiedEntityVec;

private:
	void main(float dt);
	void toggleViewPortControl(bool toControl);
	void updateMaterialMapping();
	void handleEntityValidity();
	void handleEntityHovering();
	void handleEntitySelection();
	void sandboxWindow();

	void toOutline(std::vector<entt::entity> const& entities, bool toOutline) const;

public:
	// so that all editors have access to engine interface.
	Engine& engine;
	InputManager& inputManager;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	NavMeshGeneration navMeshGenerator;
private:
	Window& window;

	GameViewPort gameViewPort;
	ComponentInspector componentInspector;
	AssetViewerUI assetViewerUi;
	AssetManagerUI assetManagerUi;
	NavigationWindow navigationWindow;

	NavBar navBar;

private:
	std::vector<entt::entity> selectedEntities;

	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;

	// This indicates whether the editor is in simulation mode.
	bool inSimulationMode;
	bool isThereChangeInSimulationMode;
};

#include "editor.ipp"
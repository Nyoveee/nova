#pragma once

#include <vector>
#include <functional>

#include "export.h"

#include "ECS/ecs.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "Assets/assetViewerUi.h"
#include "Assets/assetManagerUi.h"
#include "Assets/animationTimeline.h"
#include "Assets/animatorController.h"

#include "gameViewPort.h"
#include "editorViewPort.h"
#include "uiViewPort.h"
#include "ComponentInspection/componentInspector.h"
#include "Navigation/navMeshGeneration.h"
#include "hierarchy.h"
#include "debugUI.h"
#include "console.h"
#include "navBar.h"
#include "navigationWindow.h"

#include "type_concepts.h"
#include "Configuration/editorConfigUI.h"
#include "Configuration/gameConfigUI.h"

#include <tracyprofiler/tracy/Tracy.hpp>

using GLuint = unsigned int;

class Window;
class Engine;
class InputManager;
class AssetManager;
class ResourceManager;

enum class FocusSelectedEntity;

class Editor {
public:
	Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager, ResourceManager& resourceManager);

	~Editor();
	Editor(Editor const& other)				= delete;
	Editor(Editor&& other)					= delete;
	Editor& operator=(Editor const& other)	= delete;
	Editor& operator=(Editor&& other)		= delete;

public:
	void update(float dt);
	
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

	void displayEntityHierarchy(
		entt::registry& registry, 
		entt::entity entity, 
		bool toRecurse, 
		const char * dragAndDropIdentifier,
		std::function<void(std::vector<entt::entity>)> const& onClickFunction, 
		std::function<bool(entt::entity)> const& selectedPredicate
	);

	// editor does extra housekeeping when loading scenes (like selection of entities)
	// most editor windows should use this function instead of the scene manager's load scene function.
	void loadScene(ResourceID sceneId);

	void unpackPrefab(EntityData& entityData);

public:
	// displays a ImGui combo drop down box of all the assets related to type T.
	// first parameter is used to specific which asset id is selected.
	template <typename T, bool showNone = false>
	void displayAssetDropDownList(std::optional<ResourceID> id, const char* labelName, std::function<void(ResourceID)> const& onClickCallback);
	
	// similar to asset dropdown list, but only displays scripts the current entity has.
	void displayEntityScriptDropDownList(ResourceID id, const char* labelName, entt::entity entity, std::function<void(ResourceID)> const& onClickCallback);

	template <IsEnum T>
	void displayEnumDropDownList(T value, const char* labelName, std::function<void(T)> onClickCallback);

	// display all entities
	void displayAllEntitiesDropDownList(const char* labelName, entt::entity entity, std::function<void(entt::entity)> const& onClickCallback);

	void launchProfiler();

public:
	entt::entity hoveringEntity;
	std::vector<entt::entity> copiedEntityVec;

	bool displayingPrefabScripts = false;

private:
	void main(float dt);
	void toggleViewPortControl(bool toControl);
	void handleEntityValidity();
	void handleEntityHovering();
	void handleEntitySelection();
	void handleFocusOnSelectedEntity(FocusSelectedEntity);
	void handleUIEntitySelection();

	void toOutline(std::vector<entt::entity> const& entities, bool toOutline) const;

	void toControlMouse(bool toControl);

public:
	// so that all editors have access to engine interface.
	Engine& engine;
	InputManager& inputManager;
	AssetManager& assetManager;
	ResourceManager& resourceManager;

	NavMeshGeneration navMeshGenerator;

public:
	Window& window;

	GameViewPort gameViewPort;
	EditorViewPort editorViewPort;
	UIViewPort uiViewPort;
	ComponentInspector componentInspector;
	AssetViewerUI assetViewerUi;
	AssetManagerUI assetManagerUi;
	NavigationWindow navigationWindow;
	GameConfigUI gameConfigUI;
	EditorConfigUI editorConfigUI;
	NavBar navBar;
	AnimationTimeLine animationTimeLine;
	AnimatorController animatorController;

private:
	std::vector<entt::entity> selectedEntities;

	// This indicates whether the camera is active in the game's viewport.
	bool isControllingInViewPort;

	// This indicates whether the editor is in simulation mode.
	bool inSimulationMode;

	std::string assetSearchQuery;
	std::string uppercaseSearchQuery;
	std::string uppercaseAssetName;

	std::string entitySearchQuery;
	std::string uppercaseEntitySearchQuery;
	std::string uppercaseEntityName;

	int imguiCounter = 0;
};

#include "editor.ipp"
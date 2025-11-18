#pragma once
#include "console.h"
#include "debugUI.h"
#include "hierarchy.h"
#include "ComponentInspection/componentInspector.h"
#include "Configuration/gameConfigUI.h"
#include "Editor/Assets/animationTimeline.h"
#include "Editor/Assets/animatorController.h"
#include "Editor/navigationWindow.h"

class Editor;
class Engine;
//class Renderer;
//class ResourceManager;
class Window;

//#define ALL_WINDOWS \
//	Console, DebugUI, Hierarchy, ComponentInspector

class NavBar {
public:
	NavBar(Editor& editor);
	void update();

public:
	Engine& engine;

	Console console;
	DebugUI debugUi;
	Hierarchy hierarchyList;
	ComponentInspector componentInspector;
	GameConfigUI gameConfig;
	AnimationTimeLine& animationWindow;
	AnimatorController& animatorWindow;
	NavigationWindow& navigationWindow;

	bool consoleBool;
	bool debugUiBool;
	bool hierarchyBool;
	bool componentInspectorBool;
	bool gameConfigBool;
	bool animationBool;
	bool animatorBool;
	bool imGuiDemoBool;
	bool navigationBool;
};
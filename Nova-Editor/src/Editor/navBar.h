#pragma once
#include "console.h"
#include "debugUI.h"
#include "hierarchy.h"
#include "ComponentInspection/componentInspector.h"
#include "gameConfig.h"

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

private:
	Engine& engine;
	//Renderer& renderer;
	//ResourceManager& resourceManager;
	Console console;
	DebugUI debugUi;
	Hierarchy hierarchyList;
	ComponentInspector componentInspector;
	GameConfig gameConfig;

	bool consoleBool;
	bool debugUiBool;
	bool hierarchyBool;
	bool componentInspectorBool;
	bool gameConfigBool;


};
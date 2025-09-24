#pragma once

#include <string>

class Editor;
class NavigationSystem;
class NavMeshGeneration;

class NavigationWindow {
public:
	NavigationWindow(Editor& editor, NavigationSystem& navigationSystem, NavMeshGeneration& navMeshGenerator);

	void update();

private:
	Editor& editor;
	NavigationSystem& navigationSystem;
	NavMeshGeneration& navMeshGenerator;

	bool onFileCreate;
	int  step;
	std::string filename;
};
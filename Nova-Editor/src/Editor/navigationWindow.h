#pragma once

#include <string>

class Editor;
class NavigationSystem;
class NavMeshGeneration;
class EditorConfigUI;


using json = nlohmann::ordered_json;

class NavigationWindow {
public:
	NavigationWindow(Editor& editor, NavigationSystem& navigationSystem, NavMeshGeneration& navMeshGenerator);
	~NavigationWindow();

	void update();

private:
	Editor& editor;
	NavigationSystem& navigationSystem;
	NavMeshGeneration& navMeshGenerator;
	EditorConfigUI& editorConfigUI;

	json config;
	int selectedAgentIndex = 0;
	bool onFileCreate;
	int  step;
	std::string filename;
	std::string newAgentNameBuf;
	bool openAddAgentPopup = false;
	std::string saveFilePath = "editorConfig.json";
};
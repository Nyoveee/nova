#pragma once
#include "gameViewPort.h"

class Editor;
class Engine;

class GameConfig {
public:
	GameConfig(Editor& editor);
	void update();
	int getGameWidth();
	int getGameHeight();


private:
	Editor& editor;
	Engine& engine;
	int gameWidth;
	int gameHeight;
	char name[256];
};
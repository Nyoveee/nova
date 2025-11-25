#pragma once
#include <string>

struct GameConfig;
class Editor;

class GameConfigUI {
public:
    GameConfigUI(Editor& editor);
	~GameConfigUI();

public:
    void update();

private:
    Editor& editor;

    // Configuration data
	GameConfig& gameConfig;
};
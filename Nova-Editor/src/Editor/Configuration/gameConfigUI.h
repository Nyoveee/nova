#pragma once
#include <string>

class Editor;

class GameConfigUI {
public:
    GameConfigUI(Editor& editor);

public:
    void update();

private:
    void loadConfig();
    void saveConfig();
    void applyConfig();

private:
    Editor& editor;

    // Configuration data
    std::string windowName = "Nova Game";
    int gameWidth = 1920;
    int gameHeight = 1080;
    std::string configPath = "gameConfig.json";
};
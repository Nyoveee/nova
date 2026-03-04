#pragma once
#include <string>

class Editor;

class EditorConfigUI {
public:
    EditorConfigUI(Editor& editor);

public:
    void update();
    void loadConfig();
    void saveConfig();
    std::string getPath();

private:
    Editor& editor;
    int fontSize = 13;
    std::string configPath = "editorConfig.json";
};
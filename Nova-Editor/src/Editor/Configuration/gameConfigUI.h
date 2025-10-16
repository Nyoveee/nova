#pragma once

class Editor;

class GameConfigUI {
public:
	GameConfigUI(Editor& editor);

public:
	void update();

private:
	Editor& editor;
};
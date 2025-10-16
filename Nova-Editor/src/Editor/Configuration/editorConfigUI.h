#pragma once

class Editor;

class EditorConfigUI {
public:
	EditorConfigUI(Editor& editor);

public:
	void update();

private:
	Editor& editor;
};
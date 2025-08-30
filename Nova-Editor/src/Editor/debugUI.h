#pragma once

class Editor;
class Engine;
class Renderer;
class AssetManager;
class Window;

class DebugUI {
public:
	DebugUI(Editor& editor);

public:
	void update();

private:
	Engine& engine;
	Renderer& renderer;
	AssetManager& assetManager;
	Window& window;
};
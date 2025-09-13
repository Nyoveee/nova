#pragma once

class Editor;
class Engine;
class Renderer;
class ResourceManager;
class Window;

class DebugUI {
public:
	DebugUI(Editor& editor);

public:
	void update();

private:
	Engine& engine;
	Renderer& renderer;
	ResourceManager& resourceManager;
	Window& window;
};
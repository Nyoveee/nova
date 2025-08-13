#pragma once

class Engine;

class GameViewPort {
public:
	GameViewPort(Engine& engine);
	void update();

public:
	bool isHoveringOver;

private:
	Engine& engine;
};
#pragma once

class Engine;

class GameViewPort {
public:
	GameViewPort(Engine& engine);
	void update();

private:
	Engine& engine;
};
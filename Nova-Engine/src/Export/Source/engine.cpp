#include <iostream>

#include "engine.h"
#include "window.h"

#include "component.h"
#include "inputManager.h"

#include <GLFW/glfw3.h>

Engine::Engine(Window& window, InputManager& inputManager, int gameWidth, int gameHeight) :
	window			{ window },
	renderer		{ *this, gameWidth, gameHeight },
	cameraSystem	{ *this, inputManager },
	ecs				{},
	gameWidth		{ gameWidth },
	gameHeight		{ gameHeight }
{}

void Engine::fixedUpdate(float dt) {
	(void) dt;
}

void Engine::update(float dt) {
	cameraSystem.update(dt);
	renderer.update(dt);
}

void Engine::render(Renderer::RenderTarget target) {
	renderer.render(target);
}

int Engine::getGameWidth() const {
	return gameWidth;
}

int Engine::getGameHeight() const {
	return gameHeight;
}

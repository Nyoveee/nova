#include "engine.h"
#include "window.h"

#include "Component/component.h"
#include "inputManager.h"

#include <iostream>
#include <GLFW/glfw3.h>

Engine::Engine(Window& window, InputManager& inputManager, AssetManager& assetManager, int gameWidth, int gameHeight) :
	window			{ window },
	assetManager	{ assetManager },
	renderer		{ *this, gameWidth, gameHeight },
	cameraSystem	{ *this, inputManager },
	ecs				{},
	scriptingAPIManager{*this},
	gameWidth		{ gameWidth },
	gameHeight		{ gameHeight }
{
}


void Engine::fixedUpdate(float dt) {
	(void) dt;
}

void Engine::update(float dt) {
	cameraSystem.update(dt);
	renderer.update(dt);
	scriptingAPIManager.update();
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

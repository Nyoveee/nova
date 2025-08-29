#include "engine.h"
#include "window.h"

#include "Component/component.h"
#include "inputManager.h"

#include "assetManager.h"

#include <iostream>

Engine::Engine(Window& window, InputManager& inputManager, AssetManager& assetManager, int gameWidth, int gameHeight) :
	window					{ window },
	assetManager			{ assetManager },
	renderer				{ *this, gameWidth, gameHeight },
	cameraSystem			{ *this, inputManager },
	ecs						{ *this },
	scriptingAPIManager		{ *this },
	transformationSystem	{ ecs },
	physicsManager			{},
	gameWidth				{ gameWidth },
	gameHeight				{ gameHeight }
{}


void Engine::fixedUpdate(float dt) {
	scriptingAPIManager.update();
	physicsManager.update(dt);
}

void Engine::update(float dt) {
	cameraSystem.update(dt);
	transformationSystem.update();
	renderer.update(dt);

	assetManager.update();
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

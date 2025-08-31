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
	physicsManager			{ *this },
	gameWidth				{ gameWidth },
	gameHeight				{ gameHeight },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false }
{}

void Engine::fixedUpdate(float dt) {
	if (inSimulationMode) {
		scriptingAPIManager.update();
		physicsManager.update(dt);
	}
}

void Engine::update(float dt) {
	cameraSystem.update(dt);
	transformationSystem.update();
	renderer.update(dt);

	assetManager.update();
}

void Engine::setupSimulation() {
	// do any setup if required.
	if (!setupSimulationFunction) {
		return;
	}

	setupSimulationFunction.value()();
	setupSimulationFunction = std::nullopt;
}

void Engine::render(Renderer::RenderTarget target) {
	if (toDebugRenderPhysics) {
		physicsManager.debugRender();
	}

	renderer.render(target, toDebugRenderPhysics);
}

void Engine::startSimulation() {
	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		physicsManager.initialise();
		scriptingAPIManager.loadAllScripts();
		
		ecs.makeRegistryCopy<ALL_COMPONENTS>();

		// We set simulation mode to true to indicate that the change of simulation is successful.
		// Don't set simulation mode to true if set up falied.
		inSimulationMode = true;
	};
}

void Engine::stopSimulation() {
	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		physicsManager.clear();
		scriptingAPIManager.unloadAllScripts();

		ecs.rollbackRegistry<ALL_COMPONENTS>();
		inSimulationMode = false;
	};
}

int Engine::getGameWidth() const {
	return gameWidth;
}

int Engine::getGameHeight() const {
	return gameHeight;
}

bool Engine::isInSimulationMode() const {
	return inSimulationMode;
}

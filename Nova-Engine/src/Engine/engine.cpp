
#include <iostream>

#include "engine.h"
#include "window.h"

#include "Component/component.h"
#include "inputManager.h"
#include "assetManager.h"
#include "Debugging/Profiling.h"

#include "Serialisation/serialisation.h"

Engine::Engine(Window& window, InputManager& inputManager, AssetManager& assetManager, int gameWidth, int gameHeight) :
	window					{ window },
	assetManager			{ assetManager },
	inputManager            { inputManager },
	renderer				{ *this, gameWidth, gameHeight },
	cameraSystem			{ *this },
	ecs						{ *this },
	scriptingAPIManager		{ *this },
	transformationSystem	{ ecs },
	physicsManager			{ *this },
	gameWidth				{ gameWidth },
	gameHeight				{ gameHeight },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false }
{
}

Engine::~Engine() {
	Serialiser::serialiseComponent(ecs);
}

void Engine::fixedUpdate(float dt) {
	ZoneScoped;
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
	ZoneScoped;
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
		if (!scriptingAPIManager.loadAllScripts())
		{
			stopSimulation();
			return;
		}
		physicsManager.initialise();

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

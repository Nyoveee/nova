
#include <iostream>

#include "engine.h"
#include "window.h"

#include "ECS/component.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Profiling.h"


#include "Serialisation/serialisation.h"
#include "ECS/SceneManager.h"


Engine::Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, int gameWidth, int gameHeight) :
	window					{ window },
	resourceManager			{ resourceManager },
	inputManager            { inputManager },
	renderer				{ *this, gameWidth, gameHeight },
	cameraSystem			{ *this },
	ecs						{ *this },
	scriptingAPIManager		{ *this },
	transformationSystem	{ ecs },
	physicsManager			{ *this },
	audioSystem				{ *this },
	navigationSystem		{ *this }, 
	gameWidth				{ gameWidth },
	gameHeight				{ gameHeight },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false }
{}

Engine::~Engine() {
	stopSimulation();
	setupSimulationFunction.value()();
}

void Engine::fixedUpdate(float dt) {
	ZoneScoped;

	if (inSimulationMode) {
		scriptingAPIManager.update();
		physicsManager.update(dt);
	}
	else
	{
		scriptingAPIManager.checkModifiedScripts(dt);
	}
}

void Engine::update(float dt) {
	audioSystem.update();
	cameraSystem.update(dt);
	transformationSystem.update();
	renderer.update(dt);

	resourceManager.update();
}

void Engine::setupSimulation() {
	// do any setup if required.
	if (!setupSimulationFunction) {
		return;
	}

	setupSimulationFunction.value()();
	setupSimulationFunction = std::nullopt;
}

void Engine::render(RenderTarget target) {
	ZoneScoped;

	if (toDebugRenderPhysics) {
		physicsManager.debugRender();
	}

	renderer.render(toDebugRenderPhysics);

	if (target == RenderTarget::DefaultFrameBuffer) {
		renderer.renderToDefaultFBO();
	}
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
		audioSystem.loadAllSounds();

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
		audioSystem.unloadAllSounds();

		Serialiser::serialiseScene(ecs, "test.json");

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

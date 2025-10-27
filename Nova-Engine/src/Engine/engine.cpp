
#include <iostream>

#include "engine.h"
#include "window.h"

#include "component.h"
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
	particleSystem          { *this },
	animationSystem			{ *this },
	gameWidth				{ gameWidth },
	gameHeight				{ gameHeight },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false }
{
	std::srand(static_cast<unsigned int>(time(NULL)));
}

Engine::~Engine() {
	if (inSimulationMode) {
		stopSimulation();
		setupSimulationFunction.value()();
	}

	Serialiser::serialiseGameConfig("gameConfig.json", gameWidth, gameHeight);
}

void Engine::fixedUpdate(float dt) {
	ZoneScoped;

	if (inSimulationMode) {
		scriptingAPIManager.update();
		physicsManager.update(dt);
		navigationSystem.update(dt);
	}
}

void Engine::update(float dt) {
	ZoneScoped;

	//Note the order should be correct
	audioSystem.update();
	cameraSystem.update(dt);

	if (!inSimulationMode) {
		scriptingAPIManager.checkIfRecompilationNeeded(dt);
	}

	animationSystem.update(dt);
	transformationSystem.update();
	particleSystem.update(dt);
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

void Engine::render(RenderConfig renderConfig) {
	ZoneScoped;

	if (toDebugRenderPhysics) {
		physicsManager.debugRender();
	}

	renderer.renderMain(renderConfig);
}

void Engine::startSimulation() {
	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		animationSystem.initialiseAllControllers();

		ecs.makeRegistryCopy<ALL_COMPONENTS>();
		physicsManager.initialise();
		audioSystem.loadAllSounds();
		cameraSystem.startSimulation();
		navigationSystem.initNavMeshSystems();

		if (scriptingAPIManager.hasCompilationFailed()) {
			Logger::error("Script compilation failed. Please update them.");
			stopSimulation();
			return;
		}
		else if (!scriptingAPIManager.startSimulation()) {
			stopSimulation();
			return;
		}

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
		audioSystem.unloadAllSounds();
		cameraSystem.endSimulation();

		//Serialiser::serialiseEditorConfig("editorConfig.json");

		ecs.rollbackRegistry<ALL_COMPONENTS>();
		scriptingAPIManager.stopSimulation();

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

void Engine::SystemsOnLoad()
{
	// this->navigationSystem.initNavMeshSystems();
	
}

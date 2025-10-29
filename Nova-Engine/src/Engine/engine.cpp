
#include <iostream>

#include "engine.h"
#include "window.h"

#include "component.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Profiling.h"

#include "Serialisation/serialisation.h"
#include "ECS/SceneManager.h"


Engine::Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, GameConfig gameConfig) :
	window					{ window },
	resourceManager			{ resourceManager },
	inputManager            { inputManager },
	renderer				{ *this, gameConfig.gameWidth, gameConfig.gameHeight },
	cameraSystem			{ *this },
	ecs						{ *this },
	scriptingAPIManager		{ *this },
	transformationSystem	{ ecs },
	physicsManager			{ *this },
	audioSystem				{ *this },
	navigationSystem		{ *this }, 
	particleSystem          { *this },
	animationSystem			{ *this },
	gameConfig				{ gameConfig },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false }
{
	std::srand(static_cast<unsigned int>(time(NULL)));
	ecs.sceneManager.loadScene(gameConfig.sceneStartUp);
}

Engine::~Engine() {
	if (inSimulationMode) {
		stopSimulation();
		setupSimulationFunction.value()();
	}
}

void Engine::fixedUpdate(float dt) {
	ZoneScoped;

	physicsManager.updateTransformBodies();
	if (inSimulationMode) {
		scriptingAPIManager.update();
		physicsManager.updatePhysics(dt);
		navigationSystem.update(dt);
	}
}

void Engine::update(float dt) {
	ZoneScoped;

	//Note the order should be correct
	audioSystem.update();

	if (!inSimulationMode) {
		scriptingAPIManager.checkIfRecompilationNeeded(dt);
	}

	animationSystem.update(dt);
	particleSystem.update(dt);
	transformationSystem.update();
	cameraSystem.update(dt);
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
		physicsManager.simulationInitialise();
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
		audioSystem.unloadAllSounds();
		cameraSystem.endSimulation();

		//Serialiser::serialiseEditorConfig("editorConfig.json");
		ecs.rollbackRegistry<ALL_COMPONENTS>();
		physicsManager.resetPhysicsState();
		scriptingAPIManager.stopSimulation();

		inSimulationMode = false;
	};
}

int Engine::getGameWidth() const {
	return gameConfig.gameWidth;
}

int Engine::getGameHeight() const {
	return gameConfig.gameHeight;
}

#if 0
void Engine::setGameWidth(int value) {
	gameWidth = value;
}

void Engine::setGameHeight(int value) {
	gameHeight = value;
}
#endif

bool Engine::isInSimulationMode() const {
	return inSimulationMode;
}

void Engine::gameLockMouse(bool value) {
	isGameLockingMouse = value;

	if (!isEditorControllingMouse && isGameLockingMouse) {
		window.toEnableMouse(false);
	}
	else {
		window.toEnableMouse(true);
	}
}

void Engine::editorControlMouse(bool value) {
	isEditorControllingMouse = value;
	gameLockMouse(isGameLockingMouse);
}

void Engine::SystemsOnLoad()
{
	this->navigationSystem.initNavMeshSystems();
	this->physicsManager.systemInitialise();
}

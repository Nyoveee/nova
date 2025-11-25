
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
	toDebugRenderPhysics	{ false },
	prefabManager			{ *this },
	deltaTimeMultiplier		{ 1.f }
{
	std::srand(static_cast<unsigned int>(time(NULL)));
}

Engine::~Engine() {
	if (inSimulationMode) {
		stopSimulation();
		setupSimulationFunction.value()();
	}
}

void Engine::fixedUpdate(float dt) {
	ZoneScoped;

	if (inSimulationMode) {
		scriptingAPIManager.update();
	}
	
	physicsManager.updateTransformBodies();

	if (inSimulationMode) {
		physicsManager.updatePhysics(dt * deltaTimeMultiplier);
		navigationSystem.update(dt * deltaTimeMultiplier);
	}
}

void Engine::update(float dt) {
	ZoneScoped;

	//Note the order should be correct
	audioSystem.update();

	if (!inSimulationMode) {
		scriptingAPIManager.checkIfRecompilationNeeded(dt); // real time checking if scripts need to recompile.
	}

	animationSystem.update(dt * deltaTimeMultiplier);
	particleSystem.update(dt * deltaTimeMultiplier);
	transformationSystem.update();
	cameraSystem.update(dt); // dt is only used in editor.
	renderer.update(dt * deltaTimeMultiplier);

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
		animationSystem.initialise();

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
	inSimulationMode = false;

	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		audioSystem.unloadAllSounds();
		cameraSystem.endSimulation();
		navigationSystem.unloadNavMeshSystems();

		//Serialiser::serialiseEditorConfig("editorConfig.json");
		ecs.rollbackRegistry<ALL_COMPONENTS>();
		physicsManager.resetPhysicsState();
		scriptingAPIManager.stopSimulation();

		resourceManager.removeAllResourceInstance();

		gameLockMouse(false);
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

	// legacy reason, please don't mind the extremely confusing booleans..
	if (!isEditorControllingMouse && isGameLockingMouse) {
		window.toEnableMouse(false);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Disable);
	}
	else {
		window.toEnableMouse(true);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Enable);
	}
}

void Engine::editorControlMouse(bool value) {
	isEditorControllingMouse = value;
	gameLockMouse(isGameLockingMouse);
}

void Engine::SystemsOnLoad()
{
	resourceManager.removeAllResourceInstance();
	this->navigationSystem.initNavMeshSystems();
	this->physicsManager.systemInitialise();
}


void Engine::SystemsUnload()
{
	this->navigationSystem.unloadNavMeshSystems();

}


float Engine::getDeltaTime() const {
	return window.getDeltaTime();
}


#include <iostream>

#include "engine.h"
#include "window.h"

#include "component.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Profiling.h"

#include "Serialisation/serialisation.h"
#include "ECS/SceneManager.h"


Engine::Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, GameConfig gameConfig, RenderConfig renderConfig, State state) :
	window					{ window },
	resourceManager			{ resourceManager },
	inputManager            { inputManager },
	renderer				{ *this, renderConfig, gameConfig.gameWidth, gameConfig.gameHeight },
	cameraSystem			{ *this },
	ecs						{ *this },
	scriptingAPIManager		{ *this },
	transformationSystem	{ ecs },
	physicsManager			{ *this },
	audioSystem				{ *this },
	navigationSystem		{ *this }, 
	particleSystem          { *this },
	animationSystem			{ *this },
	uiSystem				{ *this },
	gameConfig				{ gameConfig },
	inSimulationMode		{ false },
	toDebugRenderPhysics	{ false },
	prefabManager			{ *this },
	dataManager				{ *this },
	deltaTimeMultiplier		{ 1.f },
	isPaused				{ false },
	engineState				{ state }
{
	std::srand(static_cast<unsigned int>(time(NULL)));

	if (state == State::Game) {
		scriptingAPIManager.forceLoadAssembly();
	}
}

Engine::~Engine() {
	if (inSimulationMode) {
		stopSimulation();
		setupSimulationFunction.value()();
	}
}

void Engine::fixedUpdate(float dt) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	if (isPaused) {
		return;
	}

	if (inSimulationMode) {
		scriptingAPIManager.fixedUpdate();
		physicsManager.updatePhysics(dt * deltaTimeMultiplier);
		navigationSystem.update(dt * deltaTimeMultiplier);
	}
	else {
		physicsManager.updateTransformBodies();
	}
}

void Engine::update(float dt) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	// Note the order should be correct
	audioSystem.update();

	if (!inSimulationMode) {
		scriptingAPIManager.checkIfRecompilationNeeded(dt); // real time checking if scripts need to recompile.
	}
	else {
		scriptingAPIManager.update();
	}

	if (!isPaused) {
		animationSystem.update(dt * deltaTimeMultiplier);
		particleSystem.update(dt * deltaTimeMultiplier);
	}

	transformationSystem.update();
	cameraSystem.update(dt); // dt is only used in editor.
	uiSystem.update(dt);
	
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

void Engine::render(RenderMode renderMode) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	if (toDebugRenderPhysics) {
		physicsManager.debugRender();
	}

	renderer.renderMain(renderMode);
}

void Engine::startSimulation() {
	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		if (scriptingAPIManager.hasCompilationFailed()) {
			Logger::error("Script compilation failed. Please update them.");
			stopSimulation();
			return;
		}

		inSimulationMode = true;

		ecs.recordOriginalScene();

		cameraSystem.startSimulation();

		// Load all systems that needs to be done per scene.
		SystemsOnLoad();
	};
}

void Engine::stopSimulation() {
	inSimulationMode = false;

	if (setupSimulationFunction) {
		return; // already prompted for simulation change.
	}

	setupSimulationFunction = [&]() {
		SystemsUnload();

		cameraSystem.endSimulation();

		// Manual rollback registry. Scene reverted.
		ecs.restoreOriginalScene();

		physicsManager.resetPhysicsState();
		scriptingAPIManager.stopSimulation();

		gameLockMouse(false);
		isPaused = false;

		if (engineState == State::Game) {
			window.quit();
		}
	};

	if (engineState == State::Game) {
		setupSimulationFunction.value()();
	}
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

glm::vec2 Engine::getUIMousePosition() const {
	return window.getUISpacePos();
}

void Engine::SystemsOnLoad() {
	// Make sure the next scene gets the latest script field properties in inspector to edit
	if (engineState == State::Editor)
		scriptingAPIManager.OnSceneLoaded();

	if (!inSimulationMode) {
		return;
	}

	// Initialise the animator and sequencer components..
	animationSystem.initialise();

	// Init nav mesh system.
	navigationSystem.initNavMeshSystems();

	// Create all the bodies, then run simulation setup..
	physicsManager.resetPhysicsState();
	physicsManager.simulationInitialise();

	// Load all sounds..
	audioSystem.loadAllSounds();

	if (!scriptingAPIManager.startSimulation()) {
		stopSimulation();
	}

	// unpause all systems when loaded.. and clear accumulated dt
	isPaused = false;
	window.clearAccumulatedTime();

	deltaTimeMultiplier = 1.f;
}


void Engine::SystemsUnload() {
	// Remove all created resource instances..
	resourceManager.removeAllResourceInstance();

	// Unload all sounds
	audioSystem.unloadAllSounds();

	// Unload navmesh..
	navigationSystem.unloadNavMeshSystems();

	// Reset Particle System
	particleSystem.reset();

	renderer.hdrExposure = 0.9f;
	renderer.vignette = 0.f;
}

void Engine::quit() {
	stopSimulation();
}

float Engine::getAccumulatedTime() const {
	return window.getAccumulatedTime();
}


float Engine::getDeltaTime() const {
	return window.getDeltaTime();
}

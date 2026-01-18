#include <iostream>

#include "Engine/engine.h"
#include "Engine/window.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"
#include "Editor/editor.h"
#include "Serialisation/serialisation.h"

#include <crtdbg.h>

constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (FAILED(hr)) {
		Logger::error("Failed to initialize COM {}.", static_cast<unsigned int>(hr));
		return -1;
	}

	GameConfig gameConfig = Serialiser::deserialiseGameConfig("gameConfig.json");

	// Nova Engine base applications.
	InputManager	inputManager	{};
	Window			window			{ "Nova Editor", {windowWidth, windowHeight}, gameConfig, Window::Configuration::Maximised, inputManager, Window::Viewport::Constant};
	
	ResourceManager resourceManager {};
	Engine			engine			{ window, inputManager, resourceManager, gameConfig, Engine::State::Editor };

	AssetManager	assetManager	{ resourceManager, engine };

	// Start up scene
	engine.ecs.sceneManager.loadScene(gameConfig.sceneStartUp);

	Editor			editor			{ window, engine, inputManager, assetManager, resourceManager };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			engine.render(RenderConfig::Editor);
			
			// we update the editor the simulation mode of the engine. this is because simulation may stop abruptly outside of
			// the editor's control. during simulation setup, it may fail too.
			if (!engine.isInSimulationMode() && editor.isInSimulationMode()) {
				editor.stopSimulation();
			}

			editor.update(dt);

			// every frame, check if there is a need to change simulation and initialise / clear systems.
			// this will set the simulation mode of the engine accordingly.
			engine.setupSimulation();
		}
	);
}
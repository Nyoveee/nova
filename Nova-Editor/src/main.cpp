#include <iostream>

#include "Engine/engine.h"
#include "Engine/window.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "AssetManager/assetManager.h"
#include "Editor/editor.h"

#include <crtdbg.h>

constexpr const char*	windowName		= "Nova Engine";
constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;
constexpr int			gameWidth		= 1920;
constexpr int			gameHeight		= 1080;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Nova Engine base applications.
	InputManager	inputManager	{};
	ResourceManager resourceManager {};
	Window			window			{ windowName, {windowWidth, windowHeight}, Window::Configuration::Maximised, inputManager, Window::Viewport::Constant };
	Engine			engine			{ window, inputManager, resourceManager, gameWidth, gameHeight };

	// Editor specific applications.
	AssetManager	assetManager	{ resourceManager, engine };
	Editor			editor			{ window, engine, inputManager, assetManager, resourceManager };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			engine.render(Renderer::RenderTarget::ToMainFrameBuffer);
			
			// this callback is invoked when the editor wants to change simulation mode.
			editor.update([&](bool toStartSimulation) {
				toStartSimulation ? engine.startSimulation() : engine.stopSimulation();
			});

			// every frame, check if there is a need to change simulation and initialise / clear systems.
			// this will set the simulation mode of the engine accordingly.
			engine.setupSimulation();

			// we update the editor the simulation mode of the engine. this is because simulation may stop abruptly outside of
			// the editor's control. during simulation setup, it may fail too.
			engine.isInSimulationMode() ? editor.startSimulation() : editor.stopSimulation();
		}
	);
}
#include <iostream>

#include "engine.h"
#include "window.h"
#include "inputManager.h"
#include "assetManager.h"
#include "Editor/editor.h"

#include <crtdbg.h>

constexpr const char*	windowName		= "Nova Engine";
constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;
constexpr int			gameWidth		= 1920;
constexpr int			gameHeight		= 1080;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InputManager inputManager	{};
	AssetManager assetManager   {};
	Window		 window			{ windowName, {windowWidth, windowHeight}, Window::Configuration::Maximised, inputManager, Window::Viewport::Constant };
	Engine		 engine			{ window, inputManager, assetManager, gameWidth, gameHeight };
	Editor		 editor			{ window, engine, inputManager, assetManager };

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
			engine.setupSimulation();

			// we update the editor the simulation mode of the engine. this is because simulation may stop abruptly outside of
			// the editor's control.
			engine.isInSimulationMode() ? editor.startSimulation() : editor.stopSimulation();
		}
	);
}
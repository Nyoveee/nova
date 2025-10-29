#include <iostream>
#include <crtdbg.h>

#include "Engine/engine.h"
#include "Engine/window.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Serialisation/serialisation.h"

constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	GameConfig gameConfig = Serialiser::deserialiseGameConfig("gameConfig.json");

	InputManager	inputManager	{};
	ResourceManager resourceManager	{};
	Window			window			{ gameConfig.gameName.c_str(), {windowWidth, windowHeight}, Window::Configuration::FullScreen, inputManager, Window::Viewport::ChangeDuringResize};
	Engine			engine			{ window, inputManager, resourceManager, gameConfig };

	engine.startSimulation();
	engine.setupSimulation();

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			engine.render(RenderConfig::Game);
		}
	);
}
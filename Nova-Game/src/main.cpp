#include <iostream>
#include <crtdbg.h>

#include "Engine/engine.h"
#include "Engine/window.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"

constexpr const char*	windowName		= "Nova Game";
constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;
constexpr int			gameWidth		= 1920;
constexpr int			gameHeight		= 1080;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InputManager	inputManager	{};
	ResourceManager resourceManager	{};
	Window			window			{ windowName, {windowWidth, windowHeight}, Window::Configuration::Restored, inputManager, Window::Viewport::ChangeDuringResize };
	Engine			engine			{ window, inputManager, resourceManager, gameWidth, gameHeight };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			engine.render(Engine::RenderConfig::Editor);
		}
	);
}
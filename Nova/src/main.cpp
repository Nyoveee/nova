#include <iostream>
#include <crtdbg.h>

#include "engine.h"
#include "window.h"
#include "inputManager.h"
#include "assetManager.h"

constexpr const char*	windowName		= "Nova Game";
constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;
constexpr int			gameWidth		= 1920;
constexpr int			gameHeight		= 1080;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InputManager inputManager	{};
	AssetManager assetManager   {};
	Window		 window			{ windowName, {windowWidth, windowHeight}, Window::Configuration::Restored, inputManager, Window::Viewport::ChangeDuringResize };
	Engine		 engine			{ window, inputManager, assetManager, gameWidth, gameHeight };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			engine.render(Renderer::RenderTarget::ToDefaultFrameBuffer);
		}
	);
}
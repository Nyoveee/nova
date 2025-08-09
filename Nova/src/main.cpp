#include <iostream>
#include <crtdbg.h>

#include "engine.h"
#include "window.h"
#include "inputManager.h"

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InputManager inputManager	{};
	Window		 window			{ "Nova Game", {1200, 900}, Window::Configuration::Restored, inputManager };
	Engine		 engine			{ window, inputManager };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
		}
	);
}
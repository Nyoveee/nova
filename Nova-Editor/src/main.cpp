#include <iostream>
#include <crtdbg.h>

#include "engine.h"
#include "window.h"
#include "inputManager.h"

#include "Editor/editor.h"

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	InputManager inputManager	{};
	Window		 window			{ "Nova Engine", {1200, 900}, Window::Configuration::Maximised, inputManager };
	Engine		 engine			{ window, inputManager };
	Editor		 editor			{ window, engine };

	window.run(
		// Fixed update loop
		[&](float fixedDt) {
			engine.fixedUpdate(fixedDt);
		},

		// Update loop.
		[&](float dt) {
			engine.update(dt);
			editor.update();
		}
	);
}
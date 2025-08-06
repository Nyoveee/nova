#include <iostream>

#include "../Header/engine.h"
#include "../../window.h"
#include "../../Graphics/renderer.h"

void Engine::run() {
	Window&		window		= Window::instance();
	Renderer&	renderer	= Renderer::instance();

	window.run(
		// fixed update.
		[&]() {
		},

		// normal update.
		[&]() {
			renderer.render();
		}
	);
}

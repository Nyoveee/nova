#include <iostream>

#include "../Header/engine.h"
#include "../../window.h"

void Engine::run() {
	Window& window = Window::instance();

	window.run(
		// fixed update.
		[]() {
			std::cout << "Hi, from fixed update!\n";
		},

		// normal update.
		[]() {
			std::cout << "Hi, from normal update!\n";
		}
	);
}

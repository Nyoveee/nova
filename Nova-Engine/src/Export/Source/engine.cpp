#include <iostream>

#include "../Header/engine.h"
#include "../../window.h"
#include "../../Graphics/renderer.h"
#include "../../Graphics/cameraSystem.h"

void Engine::run() {
	Window&			window			= Window::instance();
	Renderer&		renderer		= Renderer::instance();
	CameraSystem&	cameraSystem	= CameraSystem::instance();

	window.run(
		// fixed update.
		[&](float fixedDt) {
			(void) fixedDt;
		},

		// normal update.
		[&](float dt) {
			cameraSystem.update();
			renderer.update(dt);
			renderer.render();
		}
	);
}

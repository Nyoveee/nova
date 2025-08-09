#include <iostream>


#include "../Header/engine.h"
#include "../../window.h"
#include "../../Graphics/renderer.h"
#include "../../Graphics/cameraSystem.h"

#pragma comment(lib, "shlwapi.lib")

void Engine::run() {
	Window&			window			= Window::instance();
	Renderer&		renderer		= Renderer::instance();
	CameraSystem&	cameraSystem	= CameraSystem::instance();
	try{
		scriptingAPIManager.initializeScriptingAPI();
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}
	

	window.run(
		// fixed update.
		[&](float fixedDt) {
			(void) fixedDt;
		},

		// normal update.
		[&](float dt) {
			cameraSystem.update();
			scriptingAPIManager.update();
			renderer.update(dt);
			renderer.render();
			
		}
	);
	try {
		scriptingAPIManager.stopScriptingAPI();
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}
}

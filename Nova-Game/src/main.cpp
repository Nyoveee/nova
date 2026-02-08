#include <iostream>
#include <crtdbg.h>

#include "Engine/engine.h"
#include "Engine/window.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Serialisation/serialisation.h"

constexpr int			windowWidth		= 1200;
constexpr int			windowHeight	= 900;

#if defined(NDEBUG)
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
#else
int main() {
#endif
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	GameConfig		gameConfig		= Serialiser::deserialiseGameConfig("gameConfig.json");
	RenderConfig	renderConfig	= Serialiser::deserialiseRenderConfig("renderConfig.json");

	InputManager	inputManager	{};

	Window			window			{ gameConfig.gameName.c_str(), {windowWidth, windowHeight}, gameConfig, Window::Configuration::FullScreen, inputManager, Window::Viewport::ChangeDuringResize};
	
	ResourceManager resourceManager	{};
	Engine			engine			{ window, inputManager, resourceManager, gameConfig, renderConfig, Engine::State::Game };

	// In the executable, we don't do any compiling. We assume it has been compiled and provided for.
	
	// Start up scene
	engine.ecs.sceneManager.loadScene(gameConfig.sceneStartUp);
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
			engine.render(RenderMode::Game);
		}
	);
}
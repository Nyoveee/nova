#pragma once

#include <entt/entt.hpp>
#include "export.h"

class InputManager;
class Engine;
class Window;

struct Button;

class UISystem {
public:
	ENGINE_DLL_API UISystem(Engine& engine);

public:
	ENGINE_DLL_API void update(float dt);
	ENGINE_DLL_API void updateNonSimulation();
	ENGINE_DLL_API void updateSimulation(float dt);

private:
	ENGINE_DLL_API void executeButtonCallback(Button const& button, std::string const& functionName);

private:
	Engine& engine;
	InputManager& inputManager;
	Window& window;
	entt::registry& registry;

	entt::entity clickedButtonId;
	
	bool toSelect;
	bool leftMouseReleased;
};
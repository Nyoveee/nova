#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <functional>

#include "ScriptingAPIManager.h"

#include "export.h"
#include "Graphics/renderer.h"
#include "Component/ECS.h"
#include "Graphics/cameraSystem.h"
#include "Engine/transformationSystem.h"
#include "Physics/physicsManager.h"
#include "Audio/audioSystem.h"

class Window;
class Renderer;
class CameraSystem;
class ECS;
class InputManager;
class ScriptingAPIManager;
class ResourceManager;

class Engine {
public:
	DLL_API Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, int gameWidth, int gameHeight);

	DLL_API ~Engine();
	DLL_API Engine(Engine const& other)				= delete;
	DLL_API Engine(Engine&& other)					= delete;
	DLL_API Engine& operator=(Engine const& other)	= delete;
	DLL_API Engine& operator=(Engine&& other)		= delete;

public:
	DLL_API void fixedUpdate(float dt);
	DLL_API void update(float dt);
	DLL_API void render(Renderer::RenderTarget target);
	
	DLL_API void startSimulation();
	DLL_API void stopSimulation();
	DLL_API void setupSimulation();
	DLL_API bool isInSimulationMode() const;

public:
	DLL_API int getGameWidth() const;
	DLL_API int getGameHeight() const;

public:
	// allow all systems to have references of each other via the engine.
	Window&					window;
	ResourceManager&		resourceManager;
	InputManager&           inputManager;

	Renderer				renderer;
	CameraSystem			cameraSystem;
	ECS						ecs;
	ScriptingAPIManager		scriptingAPIManager;
	TransformationSystem	transformationSystem;
	PhysicsManager			physicsManager;
	AudioSystem				audioSystem;

	// allows direct modification to render debug info for physics.
	bool					toDebugRenderPhysics;

private:
	int				gameWidth;
	int				gameHeight;

	bool			inSimulationMode;

	// when transitioning between from non simulation mode to simulation mode and vice versa,
	// we will need to do some setup. 
	// this function contains the neccessary steps to setup when entering a simulation or when exiting a simulation
	// this function is assigned in the startSimulation and stopSimulation function.
	// function returns false if setup has failed.
	std::optional<std::function<void()>> setupSimulationFunction;
};
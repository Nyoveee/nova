#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <functional>

#include "ScriptingAPIManager.h"

#include "export.h"
#include "Graphics/renderer.h"
#include "ECS/ECS.h"
#include "Graphics/cameraSystem.h"
#include "Engine/transformationSystem.h"
#include "Physics/physicsManager.h"
#include "Audio/audioSystem.h"
#include "Navigation/Navigation.h"
#include "Engine/particleSystem.h"
#include "Engine/animationSystem.h"

#define _CRTDBG_MAP_ALLOC

class Window;
class InputManager;
class ScriptingAPIManager;
class ResourceManager;

class Engine {
public:
	enum class RenderConfig {
		Editor,
		Game
	};

public:
	ENGINE_DLL_API Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, int gameWidth, int gameHeight);

	ENGINE_DLL_API ~Engine();
	ENGINE_DLL_API Engine(Engine const& other)				= delete;
	ENGINE_DLL_API Engine(Engine&& other)					= delete;
	ENGINE_DLL_API Engine& operator=(Engine const& other)	= delete;
	ENGINE_DLL_API Engine& operator=(Engine&& other)		= delete;

public:
	ENGINE_DLL_API void fixedUpdate(float dt);
	ENGINE_DLL_API void update(float dt);
	
	ENGINE_DLL_API void render(RenderConfig renderConfig);
	
	ENGINE_DLL_API void startSimulation();
	ENGINE_DLL_API void stopSimulation();
	ENGINE_DLL_API void setupSimulation();
	ENGINE_DLL_API bool isInSimulationMode() const;

	
	ENGINE_DLL_API void SystemsOnLoad(); //on scene load, some system might want to reload/unload/init stuff


public:
	ENGINE_DLL_API int getGameWidth() const;
	ENGINE_DLL_API int getGameHeight() const;

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
	NavigationSystem		navigationSystem;
	ParticleSystem          particleSystem;
	AnimationSystem			animationSystem;

	// allows direct modification to render debug info for physics.
	bool					toDebugRenderPhysics;

	//allows render debug for game
	bool					toDebugRenderNavMesh;

	// allows render debug for particle emission shapes
	bool                    toDebugRenderParticleEmissionShape;

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
#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <functional>

#include "ScriptingAPIManager.h"

#include "config.h"
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
	ENGINE_DLL_API Engine(Window& window, InputManager& inputManager, ResourceManager& resourceManager, GameConfig gameConfig);

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

	ENGINE_DLL_API void setGameWidth(int value);
	ENGINE_DLL_API void setGameHeight(int value);

	ENGINE_DLL_API void editorControlMouse(bool value);
	ENGINE_DLL_API void gameLockMouse(bool value);

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

	GameConfig				gameConfig;

private:
	// Editor mouse control represents whether the editor has control over the mouse
	// The editor has the final authority over mouse control
	bool					isEditorControllingMouse = false;

	// Game Is Mouse Locked represents whether the current game wants the mouse to be locked.
	// If editor is not in control, and mouse is locked, cursor will be locked and disappear.
	bool					isGameLockingMouse		 = false;

	bool					inSimulationMode;

	// when transitioning between from non simulation mode to simulation mode and vice versa,
	// we will need to do some setup. 
	// this function contains the neccessary steps to setup when entering a simulation or when exiting a simulation
	// this function is assigned in the startSimulation and stopSimulation function.
	std::optional<std::function<void()>> setupSimulationFunction;
};
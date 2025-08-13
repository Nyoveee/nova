#pragma once

#include <vector>
#include <memory>

#include "export.h"
#include "ECS.h"

#include "Graphics/renderer.h"
#include "Graphics/cameraSystem.h"
#include "ScriptingAPIManager.h"


class Window;
class Renderer;
class CameraSystem;
class ECS;
class InputManager;
class ScriptingAPIManager;
class AssetManager;

class Engine {
public:
	DLL_API Engine(Window& window, InputManager& inputManager, AssetManager& assetManager, int gameWidth, int gameHeight);

	DLL_API ~Engine()                               = default;
	DLL_API Engine(Engine const& other)				= delete;
	DLL_API Engine(Engine&& other)					= delete;
	DLL_API Engine& operator=(Engine const& other)	= delete;
	DLL_API Engine& operator=(Engine&& other)		= delete;

public:
	DLL_API void fixedUpdate(float dt);
	DLL_API void update(float dt);
	DLL_API void render(Renderer::RenderTarget target);

public:
	DLL_API int getGameWidth() const;
	DLL_API int getGameHeight() const;

public:
	// allow all systems to have references of each other via the engine.
	Window&			window;
	AssetManager&	assetManager;
	Renderer		renderer;
	CameraSystem	cameraSystem;
	ECS				ecs;
	ScriptingAPIManager scriptingAPIManager;
private:
	int				gameWidth;
	int				gameHeight;
};
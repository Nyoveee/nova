#pragma once

#include <vector>
#include <memory>
#include "export.h"

class Window;
class Renderer;
class CameraSystem;
class ECS;

class Engine {
public:
	DLL_API Engine(Window& window);

	DLL_API ~Engine()								= default;
	DLL_API Engine(Engine const& other)				= delete;
	DLL_API Engine(Engine&& other)					= delete;
	DLL_API Engine& operator=(Engine const& other)	= delete;
	DLL_API Engine& operator=(Engine&& other)		= delete;

public:
	DLL_API void fixedUpdate(float dt);
	DLL_API void update(float dt);

private:
	Window&			window;
	Renderer&		renderer;
	CameraSystem&	cameraSystem;
	ECS&			ecs;
};
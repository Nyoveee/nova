#pragma once
class Engine;
#include <entt/entt.hpp>
#include "export.h"
class VideoSystem
{
public:
	ENGINE_DLL_API VideoSystem(Engine& engine);
	ENGINE_DLL_API VideoSystem(VideoSystem const& other) = delete;
	ENGINE_DLL_API VideoSystem(VideoSystem&& other) = delete;
	ENGINE_DLL_API VideoSystem& operator=(VideoSystem const& other) = delete;
	ENGINE_DLL_API VideoSystem& operator=(VideoSystem&& other) = delete;
	ENGINE_DLL_API void update(float dt);
	ENGINE_DLL_API void Reload();
private:
	Engine& engine;
	entt::registry& registry;
};


#pragma once
class Engine;
struct VideoPlayer;
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
	ENGINE_DLL_API bool IsVideoFinished(VideoPlayer& videoPlayer);
private:
	Engine& engine;
	entt::registry& registry;
};


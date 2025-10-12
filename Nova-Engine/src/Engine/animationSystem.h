#pragma once

#include "export.h"
#include <vector>

class Engine;
class ResourceManager;

class AnimationSystem {
public:
	ENGINE_DLL_API AnimationSystem(Engine& p_engine);

	ENGINE_DLL_API AnimationSystem(AnimationSystem const& other)			= delete;
	ENGINE_DLL_API AnimationSystem(AnimationSystem&& other)					= delete;
	ENGINE_DLL_API AnimationSystem& operator=(AnimationSystem const& other) = delete;
	ENGINE_DLL_API AnimationSystem& operator=(AnimationSystem&& other)		= delete;

public:
	ENGINE_DLL_API void update(float dt);

private:
	Engine& engine;
	ResourceManager& resourceManager;
};
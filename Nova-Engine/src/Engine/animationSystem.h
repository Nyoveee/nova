#pragma once

#include "export.h"
#include "vertex.h"
#include <vector>

#include <glm/fwd.hpp>

class Engine;
class ResourceManager;

using BoneIndex = unsigned short;

struct SkinnedMeshRenderer;

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
	ENGINE_DLL_API void calculateFinalMatrix(BoneIndex boneIndex, std::vector<Bone> const& bones, SkinnedMeshRenderer& skinnedMeshRenderer);

private:
	Engine& engine;
	ResourceManager& resourceManager;
};
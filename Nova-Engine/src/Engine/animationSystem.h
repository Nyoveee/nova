#pragma once

#include "export.h"
#include "vertex.h"
#include "controller.h"
#include <vector>

#include <glm/fwd.hpp>

class Engine;
class ResourceManager;
class Animation;

struct AnimationChannel;
struct Animator;

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
	ENGINE_DLL_API void initialiseAllControllers();
	ENGINE_DLL_API void setParameter(Animator& animator, std::string name, Controller::ParameterTypes const& value);
	
	ENGINE_DLL_API void playAnimation(Animator& animator, std::string name);

private:
	ENGINE_DLL_API void handleTransition(Animator& animator, Controller::Node const& currentNode, Controller const& controller);

private:
	ENGINE_DLL_API void updateAnimator(float dt);

	ENGINE_DLL_API void calculateFinalMatrix(ModelNodeIndex nodeIndex, glm::mat4x4 const& globalTransformationMatrix, Skeleton const& skeleton, SkinnedMeshRenderer& skinnedMeshRenderer, Animation const* animation, float timeInSeconds);
	ENGINE_DLL_API AnimationChannel const* findAnimationChannel(std::string const& nodeName, Animation const& animation);

	bool checkIfConditionFulfilled(Animator const& animator, Controller::Transition const& transition);

private:
	Engine& engine;
	ResourceManager& resourceManager;

	bool toAdvanceAnimation;
};
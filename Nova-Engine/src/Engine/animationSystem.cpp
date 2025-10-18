#include "animationSystem.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"
#include "InputManager/inputManager.h"

AnimationSystem::AnimationSystem(Engine& p_engine) : 
	engine				{ p_engine },
	resourceManager		{ p_engine.resourceManager },
	toAdvanceAnimation	{ true }
{
	InputManager& inputManager = p_engine.inputManager;

	inputManager.subscribe<ToggleAnimate>([&](auto) {
		toAdvanceAnimation = !toAdvanceAnimation;
	});
}

void AnimationSystem::update([[maybe_unused]] float dt) {
	entt::registry& registry = engine.ecs.registry;

	for (auto&& [entityId, transform, skinnedMeshRenderer] : registry.view<Transform, SkinnedMeshRenderer>().each()) {
		// retrieve skinned mesh..
		auto&& [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);
		
		if (!model || !model->skeleton) {
			continue;
		}

		Skeleton const& skeleton = model->skeleton.value();

		skinnedMeshRenderer.bonesFinalMatrices.resize(skeleton.bones.size());

		// retrieve animation...
		Animator* animator = registry.try_get<Animator>(entityId);
		Animation const* currentAnimation = nullptr;
		float timeInTicks = 0.f;

		if (animator) {
			auto&& [animation, __] = resourceManager.getResource<Model>(animator->modelId);
			
			if (animation && animation->animations.size()) {
				// advance time..
				if (toAdvanceAnimation) {
					animator->timeElapsed += dt;
				}

				currentAnimation = &animation->animations[0];
				
				if (animator->timeElapsed >= currentAnimation->durationInSeconds) {
					animator->timeElapsed = 0.f;
				}

				timeInTicks = animator->timeElapsed * currentAnimation->ticksPerSecond;
			}
		}

		// we find the root node first, and recursively calculate the final transformation matrix down...
		ModelNodeIndex rootNode = skeleton.rootNode;
		calculateFinalMatrix(rootNode, skeleton.nodes[rootNode].transformationMatrix, skeleton, skinnedMeshRenderer, currentAnimation, timeInTicks);
	}
}

// different transformation names
// -> offset matrix			: maps from model space to local bone space.
// -> transformation		: maps from local bone space to parent bone space. (chained w/ all parent node that is not a bone)
// -> globalTransformation	: maps from local bone space to model space. (if bind pose, the exact inverse of offset matrix. with animation, it will change.)
// -> finalTransformation	: globalTransformation * offset matrix. we essientially map from local, to bone, back to local.
void AnimationSystem::calculateFinalMatrix(ModelNodeIndex nodeIndex, glm::mat4x4 const& parentGlobalTransformation, Skeleton const& skeleton, SkinnedMeshRenderer& skinnedMeshRenderer, Animation const* animation, float timeInTicks) {
	ModelNode const& node = skeleton.nodes[nodeIndex];

	// calculate this node's global transformation -> which is parent's global transformation * this node's transformation.
	// however, we need to find the animation data to recalculate the children's transformation matrix.
	// this is IF this current node has a corresponding animation, AND that this animation has channels that corresponding to this node.
	glm::mat4x4 globalTransformation = [&]() {
		if (AnimationChannel const* channel = (animation ? findAnimationChannel(node.name, *animation) : nullptr)) {
			return parentGlobalTransformation * channel->getAnimatedTransform(timeInTicks);
		}
		else {
			return parentGlobalTransformation * skeleton.nodes[nodeIndex].transformationMatrix;
		}
	}();

	// calculate final transformation, if it is a bone.
	if (node.isBone) {
		glm::mat4x4 finalTransformation = globalTransformation * skeleton.bones[node.boneIndex].offsetMatrix;
		skinnedMeshRenderer.bonesFinalMatrices[node.boneIndex] = finalTransformation;
	}
		
	// recurse downwards..
	for (ModelNodeIndex childNodeIndex : node.nodeChildrens) {
		calculateFinalMatrix(childNodeIndex, globalTransformation, skeleton, skinnedMeshRenderer, animation, timeInTicks);
	}
}

AnimationChannel const* AnimationSystem::findAnimationChannel(std::string const& nodeName, Animation const& animation) {
	for (auto const& [name, channel] : animation.animationChannels) {
		if (name == nodeName) {
			return &channel;
		}
	}

	return nullptr;
}

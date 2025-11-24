#include "animationSystem.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"
#include "InputManager/inputManager.h"

#undef max
#undef min

constexpr int FPS = 60;

AnimationSystem::AnimationSystem(Engine& p_engine) : 
	engine				{ p_engine },
	resourceManager		{ p_engine.resourceManager },
	registry			{ p_engine.ecs.registry },
	toAdvanceAnimation	{ true }
{}

void AnimationSystem::update([[maybe_unused]] float dt) {
	if (engine.isInSimulationMode()) {
		updateAnimator(dt);
	}

	// =======================================================
	// We calculate the bone's final matrices here. This will be used
	// by the vertex shader for skinning.
	// =======================================================
	for (auto&& [entityId, entityData, skinnedMeshRenderer] : registry.view<EntityData, SkinnedMeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<SkinnedMeshRenderer>(entityId)) {
			continue;
		}

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
			auto&& [animation, __] = resourceManager.getResource<Model>(animator->currentAnimation);

			if (animation && animation->animations.size()) {
				currentAnimation = &animation->animations[0];
				timeInTicks = std::min(animator->timeElapsed * currentAnimation->ticksPerSecond, currentAnimation->durationInTicks - 0.01f);
			}
		}

		// we find the root node first, and recursively calculate the final transformation matrix down...
		ModelNodeIndex rootNode = skeleton.rootNode;
		calculateFinalMatrix(rootNode, skeleton.nodes[rootNode].transformationMatrix, skeleton, skinnedMeshRenderer, currentAnimation, timeInTicks);
	}
	animateSequencer(dt);
	calculateBoneMatrixes();
}

void AnimationSystem::initialise() {
	for (auto&& [entityId, animator] : registry.view<Animator>().each()) {
		initialiseAnimator(animator);
	}

	for (auto&& [entityId, sequence] : registry.view<Sequence>().each()) {
		initialiseSequence(sequence);
	}
}

void AnimationSystem::initialiseAnimator(Animator& animator) {
	animator.timeElapsed = 0.f;
	animator.currentNode = NO_CONTROLLER_NODE;
	animator.currentAnimation = TypedResourceID<Model>{ INVALID_RESOURCE_ID };
	animator.executedAnimationEvents.clear();

	auto&& [controllerPtr, _] = resourceManager.getResource<Controller>(animator.controllerId);

	if (!controllerPtr) {
		return;
	}

	animator.parameters = controllerPtr->data.parameters;
	animator.currentNode = controllerPtr->getNodes().at(ENTRY_NODE).id;
}

void AnimationSystem::initialiseSequence(Sequence& sequence) {
	sequence.timeElapsed = 0.f;
	sequence.lastTimeElapsed = 0.f;
	sequence.executedAnimationEvents.clear();
}

void AnimationSystem::handleTransition(Animator& animator, Controller::Node const& currentNode, Controller const& controller) {
	// check transition in sequence..
	for (auto&& transition : currentNode.transitions) {
		// @TODO: Handle conditions..
		if (!checkIfConditionFulfilled(animator, transition)) {
			continue;
		}

		// We found a valid transition..
		auto iterator = controller.data.nodes.find(transition.nextNode);

		if (iterator == controller.data.nodes.end()) {
			assert(false && "Transition pointing to an invalid node?");
			continue;
		}

		animator.currentNode = transition.nextNode;
		animator.timeElapsed = 0;
		animator.currentAnimation = iterator->second.animation;
		animator.executedAnimationEvents.clear();
		return;
	}

	// no valid transition found..
	if (currentNode.toLoop) {
		animator.timeElapsed = 0;
		animator.executedAnimationEvents.clear();
	}
}

void AnimationSystem::setParameter(Animator& animator, std::string name, Controller::ParameterTypes const& value){
	auto&& iter = std::find_if(std::begin(animator.parameters), std::end(animator.parameters), [&](auto&& parameter) {return parameter.name == name; });
	if (iter != std::end(animator.parameters)) {
		std::visit(
			[&](auto& param,auto const& setterValue) {
				using ParameterType = std::decay_t<decltype(param)>;
				using SetterType = std::decay_t<decltype(setterValue)>;
				if constexpr (std::is_same_v<ParameterType, SetterType>)
					param = setterValue;
				else
					Logger::warn("Attempting to set animator parameter of a different type");
			}
		, iter->value,value);
	}
	else
	{
		Logger::warn("Attempting to set animator parameter with invalid name");
	}
}

void AnimationSystem::playAnimation(Animator& animator, std::string name) {
	auto&& [controller, _] = resourceManager.getResource<Controller>(animator.controllerId);
	
	if (!controller) {
		Logger::warn("Invalid controller.");
		return;
	}

	// Attempt to find the animation..
	auto iterator = std::ranges::find_if(controller->data.nodes, [&](auto const& pair) {
		Controller::Node const& node = pair.second;
		return node.name == name;
	});

	if (iterator == controller->data.nodes.end()) {
		Logger::warn("Invalid animation name.");
		return;
	}

	animator.currentNode = iterator->first;
	animator.timeElapsed = 0;
	animator.currentAnimation = iterator->second.animation;
	animator.executedAnimationEvents.clear();
}

void AnimationSystem::updateSequencer(entt::entity entityId, Sequence& sequence, Sequencer& sequencer, float dt) {
	if (sequence.currentFrame < sequencer.data.lastFrame) {
		sequence.timeElapsed += dt;
		sequence.currentFrame = static_cast<int>(sequence.timeElapsed * FPS);
	}

	for (auto&& animationEvent : sequencer.data.animationEvents) {
		if (sequence.currentFrame > animationEvent.key && !sequence.executedAnimationEvents.count(animationEvent.key)) {
			sequence.executedAnimationEvents.insert(animationEvent.key);
			engine.scriptingAPIManager.executeFunction(entityId, animationEvent.scriptId, animationEvent.functionName);
		}
	};

	if (sequence.currentFrame >= sequencer.data.lastFrame) {
		sequence.currentFrame = sequencer.data.lastFrame;

		if (sequence.toLoop) {
			sequence.currentFrame = 0;
			sequence.timeElapsed = 0.f;
			sequence.executedAnimationEvents.clear();
		}
	}
}

void AnimationSystem::animateSequencer(float dt) {
	for (auto&& [entityId, entityData, transform, sequence] : registry.view<EntityData, Transform, Sequence>().each()) {
		if (!entityData.isActive) {
			continue;
		}

		auto&& [sequencer, _] = resourceManager.getResource<Sequencer>(sequence.sequencerId);

		if (!sequencer) {
			continue;
		}

		if (engine.isInSimulationMode()) {
			updateSequencer(entityId, sequence, *sequencer, dt);
		}

		if (sequence.timeElapsed == sequence.lastTimeElapsed) {
			continue;
		}

		sequence.lastTimeElapsed = sequence.timeElapsed;

		// We do actually lerping here..
		sequencer->setInterpolatedTransform(sequence.currentFrame, transform);
	}
}

void AnimationSystem::updateAnimator(float dt) {
	// =======================================================
	// We first update all our animator components, handling it's state
	// in the animation controller node graphs
	// =======================================================

	for (auto&& [entityId, entityData, animator] : registry.view<EntityData, Animator>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Animator>(entityId)) {
			continue;
		}

		// Get controller resource..
		auto&& [controllerPtr, _] = resourceManager.getResource<Controller>(animator.controllerId);

		if (!controllerPtr) {
			continue;
		}

		Controller const& controller = *controllerPtr;

		auto&& nodes = controller.getNodes();
		auto iterator = nodes.find(animator.currentNode);
		
		// if current node is invalid, we do nothing.. (this shouldn't happen)
		if (iterator == nodes.end()) {
			assert(false && "Animator's current node is pointing to an invalid node?");
			continue;
		}

		// set the current animation..
		auto&& [id, currentNode] = *iterator;

		// if current node is entry node..
		if (id == ENTRY_NODE) {
			handleTransition(animator, currentNode, controller);
			continue;
		}

		// retrieve the current animation based on current node..
		auto&& [animation, __] = resourceManager.getResource<Model>(animator.currentAnimation);

		if (!animation || animation->animations.empty()) {
			continue;
		}

		if (animator.timeElapsed >= animation->animations[0].durationInSeconds) {
			animator.timeElapsed = animation->animations[0].durationInSeconds;
			handleTransition(animator, currentNode, controller);
		}
		else {
			// advance time..
			animator.timeElapsed += dt;

			// trigger animation event if possible..
			int frameIndex = static_cast<int>(animator.timeElapsed * animation->animations[0].ticksPerSecond);

			for (auto&& animationEvent : currentNode.animationEvents) {
				if (frameIndex > animationEvent.key && !animator.executedAnimationEvents.count(animationEvent.key)) {
					animator.executedAnimationEvents.insert(animationEvent.key);
					engine.scriptingAPIManager.executeFunction(entityId, animationEvent.scriptId, animationEvent.functionName);
				}
			};
		}
	}
}

void AnimationSystem::calculateBoneMatrixes() {
	// =======================================================
	// We calculate the bone's final matrices here. This will be used
	// by the vertex shader for skinning.
	// =======================================================
	for (auto&& [entityId, entityData, skinnedMeshRenderer] : registry.view<EntityData, SkinnedMeshRenderer>().each()) {
		if (!entityData.isActive) {
			continue;
		}

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
			auto&& [animation, __] = resourceManager.getResource<Model>(animator->currentAnimation);

			if (animation && animation->animations.size()) {
				currentAnimation = &animation->animations[0];
				timeInTicks = std::min(animator->timeElapsed * currentAnimation->ticksPerSecond, currentAnimation->durationInTicks - 0.01f);
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
		//skinnedMeshRenderer.bonesFinalMatrices[node.boneIndex] = glm::mat4{1.f};
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

bool AnimationSystem::checkIfConditionFulfilled(Animator const& animator, Controller::Transition const& transition) {
	// all conditions have to be fulfilled.
	for (auto&& condition : transition.conditions) {
		// find the value to compare with from animator..
		auto iterator = std::ranges::find_if(animator.parameters, [&](auto&& parameter) {
			return parameter.name == condition.name;
		});

		// if doesn't match (shouldn't really happen though, we fail immediately.)
		if (iterator == animator.parameters.end()) {
			return false;
		}

		Controller::Parameter const& animatorParameter = *iterator;

		bool checkResult = std::visit([&](auto&& animatorValue) {
			// animator's parameter should be the exact same type as condition.
			using T = std::decay_t<decltype(animatorValue)>;
			T const& testValue = std::get<T>(condition.value);

			// we now run the respective checks..
			// for boolean, its a simple check if its the same
			if constexpr (std::same_as<T, bool>) {
				return animatorValue == testValue;
			}
			else {
				switch (condition.check)
				{
					using enum Controller::Condition::Check;
				case Greater:
					return animatorValue >= testValue;
				case Lesser:
					return animatorValue <= testValue;
				case Equal:
					return animatorValue == testValue;
				case NotEqual:
					return animatorValue != testValue;
				default:
					assert(false && "Unhandled check.");
					return false;
				}
			}
		}, animatorParameter.value);

		// failed the check..
		if (!checkResult) {
			return false;
		}
	}

	return true;
}

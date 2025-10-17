#pragma once

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

#include <string>
#include <unordered_map>

#include "reflection.h"
#include "export.h"

// a key represents a keyframe in the animation channel.
// this struct follows the same layout as the assimp counterpart.
struct VectorKey {
	float key;
	glm::vec3 vector3;

	REFLECTABLE(
		key,
		vector3
	)
};

struct QuatKey {
	float key;
	glm::quat quaternion;

	REFLECTABLE(
		key,
		quaternion
	)
};

// an animation channel relates to a bone. it contains all the keys for translation, scaling and rotation.
struct AnimationChannel {
	std::vector<VectorKey>	positions;
	std::vector<QuatKey>	rotations;
	std::vector<VectorKey>	scalings;

	REFLECTABLE(
		positions,
		rotations,
		scalings
	)
};

class Animation {
public:
	FRAMEWORK_DLL_API Animation() = default;
	FRAMEWORK_DLL_API Animation(std::string name, float durationInTicks, float ticksPerSecond, std::unordered_map<std::string, AnimationChannel> animationChannels);

public:
	FRAMEWORK_DLL_API glm::mat4x4 getAnimatedTransformation(float time);

public:
	std::string name;
	float durationInTicks;		// total amount of ticks.
	float ticksPerSecond;		// animation FPS

	float durationInSeconds;

	std::unordered_map<std::string, AnimationChannel> animationChannels;

	REFLECTABLE(
		name,
		durationInTicks,
		ticksPerSecond,
		durationInSeconds,
		animationChannels
	)
};
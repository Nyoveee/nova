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

	glm::mat4x4 getAnimatedTransform(float timeInTicks) const {
		// Get the new local position, rotation and scale.
		glm::vec3 position = getInterpolatedPosition(timeInTicks);
		glm::quat rotation = glm::normalize(getInterpolatedRotation(timeInTicks));
		glm::vec3 scale    = getInterpolatedScale(timeInTicks);

		// Recalculate the transform matrix.
		glm::mat4x4 animatedTransform { 1 };
		animatedTransform = glm::translate(animatedTransform, position);
		animatedTransform = animatedTransform * glm::mat4_cast(rotation);
		animatedTransform = glm::scale(animatedTransform, scale);

		return animatedTransform;
	}

	template <typename Container>
	int findIndex(float timeInTicks, Container const& container) const {
		// We need to first find our next key..
		for (int i = 0; i < container.size() - 1; ++i) {
			if (timeInTicks < container[i + 1].key) {
				return i;
			}
		}

		assert(false && "no key found?");
		return static_cast<int>(container.size()) - 2;
	}

	// calculate the appropriate lerp factor based on the current time, the first key and the 2nd (next) key.
	float lerpFactor(float timeInTicks, float firstKeyTime, float secondKeyTime) const {
		float timeElapsed	= timeInTicks	- firstKeyTime;		
		float totalTime		= secondKeyTime - firstKeyTime;
		float factor		= timeElapsed	/ totalTime;
		
		assert(factor >= 0.f && factor <= 1.f);
		return factor;
	}

	glm::vec3 getInterpolatedPosition(float timeInTicks) const {
		if (positions.size() == 1) {
			return positions[0].vector3;
		}

		int positionIndex = findIndex(timeInTicks, positions);
		int nextPositionIndex = positionIndex + 1;

		float factor = lerpFactor(timeInTicks, positions[positionIndex].key, positions[nextPositionIndex].key);
		return glm::mix(positions[positionIndex].vector3, positions[nextPositionIndex].vector3, factor);
	}

	glm::quat getInterpolatedRotation(float timeInTicks) const {
		if (rotations.size() == 1) {
			return rotations[0].quaternion;
		}

		int rotationIndex = findIndex(timeInTicks, rotations);
		int nextRotationIndex = rotationIndex + 1;

		float factor = lerpFactor(timeInTicks, rotations[rotationIndex].key, rotations[nextRotationIndex].key);
		return glm::slerp(rotations[rotationIndex].quaternion, rotations[nextRotationIndex].quaternion, factor);
	}

	glm::vec3 getInterpolatedScale(float timeInTicks) const {
		if (scalings.size() == 1) {
			return scalings[0].vector3;
		}

		int scaleIndex = findIndex(timeInTicks, scalings);
		int nextScaleIndex = scaleIndex + 1;

		float factor = lerpFactor(timeInTicks, scalings[scaleIndex].key, scalings[nextScaleIndex].key);
		return glm::mix(scalings[scaleIndex].vector3, scalings[nextScaleIndex].vector3, factor);
	}
};

class Animation {
public:
	FRAMEWORK_DLL_API Animation() = default;
	FRAMEWORK_DLL_API Animation(std::string name, float durationInTicks, float ticksPerSecond, std::unordered_map<std::string, AnimationChannel> animationChannels);

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
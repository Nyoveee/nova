#include "nova_math.h"
#include "sequencer.h"
#include <glm/gtx/spline.hpp> // For glm::catmullRom

template<typename T>
void Sequencer::recordKeyframe(int currentFrame, T const& keyframeData, std::vector<Keyframe<T>>& keyframes) {
	auto it = keyframes.begin();

	for (; it != keyframes.end();) {
		Keyframe<T> const& keyframe = *it;

		if (keyframe.frame == currentFrame) {
			// We remove this keyframe..
			it = keyframes.erase(it);
			break;
		}
		else if (keyframe.frame < currentFrame) {
			++it;
			continue;
		}

		// we found a spot to insert..
		break;
	}

	Keyframe<T> newKeyframe{
		.frame = currentFrame,
		.lerpType = LerpType::CatmullRom,
		.power = 1.f,
		.data = keyframeData,
	};

	if (currentFrame > data.lastFrame) {
		data.lastFrame = currentFrame;
	}

	keyframes.insert(it, std::move(newKeyframe));
}

template <typename T>
void Sequencer::setInterpolatedData(int currentFrame, T& data, std::vector<Keyframe<T>> const& keyframes) {
	if (keyframes.empty() || currentFrame < keyframes[0].frame) {
		return;
	}

	int previousFrameIndex = -1;
	int nextFrameIndex = -1;

	// for catmull rom spline..
	int twoFrameBeforeIndex = -1;
	int twoFrameAfterIndex = -1;

	// Find the 2 in between keyframes..
	for (int i = 0; i < keyframes.size() - 1; ++i) {
		if (currentFrame <= keyframes[i + 1].frame) {
			previousFrameIndex = i;
			nextFrameIndex = i + 1;

			if (i == 0) {
				twoFrameBeforeIndex = previousFrameIndex;
			}
			else {
				twoFrameBeforeIndex = previousFrameIndex - 1;
			}

			if (nextFrameIndex == keyframes.size() - 1) {
				twoFrameAfterIndex = nextFrameIndex;
			}
			else {
				twoFrameAfterIndex = nextFrameIndex + 1;
			}

			break;
		}
	}

	if (previousFrameIndex == -1) {
		return;
	}

	Keyframe<T> const& previousKeyframe = keyframes[previousFrameIndex];
	Keyframe<T> const& nextKeyframe = keyframes[nextFrameIndex];

	Keyframe<T> const& twoFrameBeforeKeyframe = keyframes[twoFrameBeforeIndex];
	Keyframe<T> const& twoFrameAfterKeyframe = keyframes[twoFrameAfterIndex];

	// Get lerp factor..
	int frameDifference = nextKeyframe.frame - previousKeyframe.frame;
	assert(frameDifference > 0 && "Keyframes are not ordered.");

	int frameProgressed = currentFrame - previousKeyframe.frame;
	assert(frameProgressed >= 0 && "Current frame is smaller than previous key frame");

	float lerpFactor = (float)frameProgressed / (float)frameDifference;
	assert(lerpFactor >= 0 && lerpFactor <= 1 && "Lerp factor invalid. Current frame probably exceeded next frame.");

	// We do the lerping..
	switch (nextKeyframe.lerpType)
	{
	case LerpType::Linear:
		if constexpr (std::same_as<T, glm::vec3>) {
			data = glm::mix(previousKeyframe.data, nextKeyframe.data, std::powf(lerpFactor, nextKeyframe.power));
		}
		else if constexpr (std::same_as<T, glm::quat>) {
			data = glm::slerp(previousKeyframe.data, nextKeyframe.data, std::powf(lerpFactor, nextKeyframe.power));
		}
		else {
			// static_assert(False<T>, "Unhandled case.");
		}
		break;

	case LerpType::Smooth:
		if constexpr (std::same_as<T, glm::vec3>) {
			data = glm::mix(previousKeyframe.data, nextKeyframe.data, std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		}
		else if constexpr (std::same_as<T, glm::quat>) {
			data = glm::slerp(previousKeyframe.data, nextKeyframe.data, std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		}
		else {
			// static_assert(False<T>, "Unhandled case.");
		}
		
		break;
	case LerpType::CatmullRom:
		if constexpr (std::same_as<T, glm::vec3>) {
			data = glm::catmullRom(twoFrameBeforeKeyframe.data, previousKeyframe.data, nextKeyframe.data, twoFrameAfterKeyframe.data, lerpFactor);
		}
		else if constexpr (std::same_as<T, glm::quat>) {
			data = glm::slerp(previousKeyframe.data, nextKeyframe.data, std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		}
		else {
			// static_assert(False<T>, "Unhandled case.");
		}
		break;
	}
}
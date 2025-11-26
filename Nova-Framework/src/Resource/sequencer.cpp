#include "sequencer.h"
#include "component.h"
#include "nova_math.h"
#include "Logger.h"

#include <ranges>

#include <glm/gtx/spline.hpp> // For glm::catmullRom

Sequencer::Sequencer(ResourceID id, ResourceFilePath filePath, Data data) :
	Resource		{ id, std::move(filePath) },
	data			{ std::move(data) }
{}

void Sequencer::recordKeyframe(int currentFrame, Transform const& transform) {
	auto it = data.keyframes.begin();

	for (; it != data.keyframes.end();) {
		Keyframe const& keyframe = *it;

		if (keyframe.frame == currentFrame) {
			// We remove this keyframe..
			it = data.keyframes.erase(it);
			break;
		}
		else if (keyframe.frame < currentFrame) {
			++it;
			continue;
		}

		// we found a spot to insert..
		break;
	}
	
	Keyframe newKeyframe{
		.frame			= currentFrame,
		.lerpType		= Keyframe::LerpType::CatmullRom,
		.power			= 1.f,
		.localPosition	= transform.localPosition,
		.localScale		= transform.localScale,
		.localRotation	= transform.localRotation
	};

	if (currentFrame > data.lastFrame) {
		data.lastFrame = currentFrame;
	}

	data.keyframes.insert(it, std::move(newKeyframe));
}

void Sequencer::setInterpolatedTransform(int currentFrame, Transform& transform) {
	if (data.keyframes.empty() || currentFrame < data.keyframes[0].frame) {
		return;
	}

	int previousFrameIndex = -1;
	int nextFrameIndex = -1;

	// for catmull rom spline..
	int twoFrameBeforeIndex = -1;
	int twoFrameAfterIndex = -1;

	// Find the 2 in between keyframes..
	for (int i = 0; i < data.keyframes.size() - 1; ++i) {
		if (currentFrame <= data.keyframes[i + 1].frame) {
			previousFrameIndex = i;
			nextFrameIndex = i + 1;

			if (i == 0) {
				twoFrameBeforeIndex = previousFrameIndex;
			}
			else {
				twoFrameBeforeIndex = previousFrameIndex - 1;
			}
			
			if (nextFrameIndex == data.keyframes.size() - 1) {
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

	Keyframe const& previousKeyframe = data.keyframes[previousFrameIndex];
	Keyframe const& nextKeyframe = data.keyframes[nextFrameIndex];

	Keyframe const& twoFrameBeforeKeyframe = data.keyframes[twoFrameBeforeIndex];
	Keyframe const& twoFrameAfterKeyframe = data.keyframes[twoFrameAfterIndex];

	// Get lerp factor..
	int frameDifference = nextKeyframe.frame - previousKeyframe.frame;
	assert(frameDifference > 0 && "Keyframes are not ordered.");

	int frameProgressed = currentFrame - previousKeyframe.frame;
	assert(frameProgressed >= 0 && "Current frame is smaller than previous key frame");

	float lerpFactor = (float) frameProgressed / (float) frameDifference;
	assert(lerpFactor >= 0 && lerpFactor <= 1 && "Lerp factor invalid. Current frame probably exceeded next frame.");

	// We do the lerping..
	switch (nextKeyframe.lerpType)
	{
	case Keyframe::LerpType::Linear:
		transform.localPosition = glm::mix  (previousKeyframe.localPosition,	nextKeyframe.localPosition, std::powf(lerpFactor, nextKeyframe.power));
		transform.localScale	= glm::mix  (previousKeyframe.localScale,		nextKeyframe.localScale,	std::powf(lerpFactor, nextKeyframe.power));
		transform.localRotation = glm::slerp(previousKeyframe.localRotation,	nextKeyframe.localRotation, std::powf(lerpFactor, nextKeyframe.power));
		break;

	case Keyframe::LerpType::Smooth:
		transform.localPosition = glm::mix  (previousKeyframe.localPosition,	nextKeyframe.localPosition, std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		transform.localScale	= glm::mix  (previousKeyframe.localScale,		nextKeyframe.localScale,	std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		transform.localRotation = glm::slerp(previousKeyframe.localRotation,	nextKeyframe.localRotation, std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power));
		break;
	case Keyframe::LerpType::CatmullRom:
		transform.localPosition = glm::catmullRom  (twoFrameBeforeKeyframe.localPosition,	previousKeyframe.localPosition, nextKeyframe.localPosition, twoFrameAfterKeyframe.localPosition, lerpFactor);
		transform.localScale	= glm::catmullRom  (twoFrameBeforeKeyframe.localScale,		previousKeyframe.localScale,	nextKeyframe.localScale,	twoFrameAfterKeyframe.localScale,	 lerpFactor);
		transform.localRotation = glm::slerp	   (previousKeyframe.localRotation,	nextKeyframe.localRotation, Math::smoothstep(lerpFactor));
		break;
	}

}

#if false
glm::vec3 Sequencer::getInterpolatedPosition() {

}

glm::vec3 Sequencer::getInterpolatedScale() {

}

glm::quat Sequencer::getInterpolatedRotation() {

}
#endif
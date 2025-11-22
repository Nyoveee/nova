#include "sequencer.h"
#include "component.h"
#include "nova_math.h"
#include "Logger.h"
#include <ranges>

Sequencer::Sequencer(ResourceID id, ResourceFilePath filePath, Data data) :
	Resource		{ id, std::move(filePath) },
	data			{ std::move(data) }
{}

void Sequencer::recordKeyframe(int currentFrame, Transform const& transform) {
	auto it = data.keyframes.begin();

	for (; it != data.keyframes.end();) {
		Keyframe const& keyframe = *it;

		if (keyframe.frame == currentFrame) {
			Logger::warn("Attempting to record keyframe that already exist.");
			return;
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
		.lerpType		= Keyframe::LerpType::Smooth,
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

	// Find the 2 in between keyframes..
	for (int i = 0; i < data.keyframes.size() - 1; ++i) {
		if (currentFrame <= data.keyframes[i + 1].frame) {
			previousFrameIndex = i;
			nextFrameIndex = i + 1;
			break;
		}
	}

	if (previousFrameIndex == -1) {
		return;
	}

	Keyframe const& previousKeyframe = data.keyframes[previousFrameIndex];
	Keyframe const& nextKeyframe = data.keyframes[nextFrameIndex];
	
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
		lerpFactor = std::powf(lerpFactor, nextKeyframe.power);
		break;
	case Keyframe::LerpType::Smooth:
		lerpFactor = std::powf(Math::smoothstep(lerpFactor), nextKeyframe.power);
		break;
	//case Keyframe::LerpType::Sine:
		//lerpFactor = std::powf(Math::sinestep(lerpFactor), nextKeyframe.power);
		//break;
	}
	
	transform.localPosition = glm::mix  (previousKeyframe.localPosition,	nextKeyframe.localPosition, lerpFactor);
	transform.localScale	= glm::mix  (previousKeyframe.localScale,		nextKeyframe.localScale,	lerpFactor);
	transform.localRotation = glm::slerp(previousKeyframe.localRotation,	nextKeyframe.localRotation, lerpFactor);
}

#if false
glm::vec3 Sequencer::getInterpolatedPosition() {

}

glm::vec3 Sequencer::getInterpolatedScale() {

}

glm::quat Sequencer::getInterpolatedRotation() {

}
#endif
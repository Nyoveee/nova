#include "animation.h"

Animation::Animation(std::string name, float durationInTicks, float ticksPerSecond, std::unordered_map<std::string, AnimationChannel> animationChannels) :
	name				{ std::move(name) },
	durationInTicks		{ durationInTicks },
	ticksPerSecond		{ ticksPerSecond },
	durationInSeconds	{ durationInTicks / ticksPerSecond },
	animationChannels	{ std::move(animationChannels) }
{}

glm::mat4x4 Animation::getAnimatedTransformation([[maybe_unused]] float time) {
	return glm::mat4x4();
}

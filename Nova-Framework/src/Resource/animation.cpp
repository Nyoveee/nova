#include "animation.h"

Animation::Animation(std::string name, float durationInTicks, float ticksPerSecond, std::unordered_map<std::string, AnimationChannel> animationChannels) :
	name				{ std::move(name) },
	durationInTicks		{ durationInTicks },
	ticksPerSecond		{ ticksPerSecond },
	durationInSeconds	{ durationInTicks / ticksPerSecond },
	animationChannels	{ std::move(animationChannels) }
{}

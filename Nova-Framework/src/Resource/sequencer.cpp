#include "sequencer.h"
#include "component.h"
#include "nova_math.h"
#include "Logger.h"

#include <ranges>


Sequencer::Sequencer(ResourceID id, ResourceFilePath filePath, Data data) :
	Resource		{ id, std::move(filePath) },
	data			{ std::move(data) }
{}


void Sequencer::setInterpolatedTransform(int currentFrame, Transform& transform) {
	setInterpolatedData(currentFrame, transform.localPosition,	data.positionKeyframes);
	setInterpolatedData(currentFrame, transform.localScale,		data.scaleKeyframes);
	setInterpolatedData(currentFrame, transform.localRotation,	data.rotationKeyframes);
}

#if false
glm::vec3 Sequencer::getInterpolatedPosition() {

}

glm::vec3 Sequencer::getInterpolatedScale() {

}

glm::quat Sequencer::getInterpolatedRotation() {

}
#endif
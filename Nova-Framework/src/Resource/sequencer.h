#pragma once

#include "animationEvent.h"
#include "reflection.h"
#include "resource.h"

#include <vector>

struct Transform;

// All sequencer are 60 FPS.
class Sequencer : public Resource {
public:
	struct Keyframe {
		int frame;

		enum class LerpType {
			Linear,
			Smooth
			//Sine
		} lerpType = LerpType::Smooth;

		float power = 1.f;

		glm::vec3 localPosition {};
		glm::vec3 localScale {};
		glm::quat localRotation {};

		REFLECTABLE(
			frame,
			power,
			lerpType,
			localPosition,
			localScale,
			localRotation
		)

		int copyFrame = -1;
	};

	struct Data {
		std::vector<Keyframe>		keyframes		{};
		std::vector<AnimationEvent> animationEvents	{};
		int lastFrame								= 1;

		REFLECTABLE(
			keyframes,
			animationEvents,
			lastFrame
		)
	};

public:
	FRAMEWORK_DLL_API Sequencer(ResourceID id, ResourceFilePath filePath, Data data);
	
public:
	FRAMEWORK_DLL_API void recordKeyframe(int currentFrame, Transform const& transform);
	
	//@TODO: Template-ify and support many components..
	FRAMEWORK_DLL_API void setInterpolatedTransform(int currentFrame, Transform& transform);

public:
	Data data;
};

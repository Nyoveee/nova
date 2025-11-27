#pragma once

#include "animationEvent.h"
#include "reflection.h"
#include "resource.h"

#include <vector>

struct Transform;

// All sequencer are 60 FPS.
class Sequencer : public Resource {
public:
	enum class LerpType {
		Linear,
		Smooth,
		CatmullRom
	};

	template <typename T>
	struct Keyframe {
		int frame;
		LerpType lerpType = LerpType::CatmullRom;
		float power = 1.f;

		T data {};

		REFLECTABLE(
			frame,
			power,
			lerpType,
			data
		)

		int copyFrame = -1;
	};

	struct Data {
		std::vector<Keyframe<glm::vec3>>		positionKeyframes	{};
		std::vector<Keyframe<glm::vec3>>		scaleKeyframes		{};
		std::vector<Keyframe<glm::quat>>		rotationKeyframes	{};
		std::vector<AnimationEvent>				animationEvents		{};
		int lastFrame												= 1;

		REFLECTABLE(
			positionKeyframes,
			scaleKeyframes,
			rotationKeyframes,
			animationEvents,
			lastFrame
		)
	};

public:
	FRAMEWORK_DLL_API Sequencer(ResourceID id, ResourceFilePath filePath, Data data);
	
public:
	template <typename T>
	void recordKeyframe(int currentFrame, T const& keyframeData, std::vector<Keyframe<T>>& keyframes);
	
	//@TODO: Template-ify and support many components..
	FRAMEWORK_DLL_API void setInterpolatedTransform(int currentFrame, Transform& transform);

	template <typename T>
	void setInterpolatedData(int currentFrame, T& data, std::vector<Keyframe<T>> const& keyframes);

public:
	Data data;
};

#include "sequencer.ipp"
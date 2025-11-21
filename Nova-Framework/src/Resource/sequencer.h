#pragma once

#include "reflection.h"
#include "resource.h"

class Sequencer : public Resource {
public:
	struct Keyframe {
		float key; // seconds;
		glm::vec3 localPosition;
		glm::vec3 localScale;
		glm::quat localRotation;

		REFLECTABLE(
			key,
			localPosition,
			localScale,
			localRotation
		)
	};

	struct Data {
		std::vector<Keyframe> keyframes {};
		float totalDuration				= 0.f;
		int fps							= 60;

		REFLECTABLE(
			keyframes,
			totalDuration,
			fps
		)
	};

public:
	Sequencer(ResourceID id, ResourceFilePath filePath, Data data);
	
public:
	Data data;
};

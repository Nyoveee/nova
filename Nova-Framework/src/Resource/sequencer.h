#pragma once

#include "resource.h"

// ALL SEQUENCERS ARE 30 FPS.
// We work with real time (seconds).
class Sequencer : public Resource {
public:
	struct Keyframe {
		float key; // seconds;
		glm::vec3 position;
		glm::vec3 scale;
		glm::quat rotation;
	};

public:
	Sequencer(ResourceID id, ResourceFilePath filePath);

private:
	std::vector<Keyframe> keyframes;
	float totalDuration;
};

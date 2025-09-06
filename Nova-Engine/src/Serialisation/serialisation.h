#pragma once

class ECS;

namespace Serialiser {
	void serialiseComponent(ECS& ecs);
	void deserialiseComponent(ECS& ecs);
};
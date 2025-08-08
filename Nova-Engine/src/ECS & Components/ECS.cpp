#include "ECS.h"

ECS::ECS() : registry{} {}

ECS& ECS::instance() {
	static ECS ecs{};
	return ecs;
}

ECS::~ECS() {
	//registry.destroy();
}

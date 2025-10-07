#pragma once

#include <glm/vec3.hpp>
#include <entt/entt.hpp>

struct PhysicsRay {
	glm::vec3 origin;
	glm::vec3 direction;
};

struct PhysicsRayCastResult {
	entt::entity entity;
	glm::vec3 point;
};
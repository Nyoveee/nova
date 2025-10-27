#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>


//A list of events that can be called

struct  TransformUpdateEvent
{
	entt::entity entityID;
	glm::vec3 previousPosition;
	glm::vec3 previousScale;
	glm::quat previousRotation;

};

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class ECS;

class TransformationSystem {
public:
	TransformationSystem(ECS& ecs);

public:
	void update();

public:
	struct DecomposedMatrix {
		glm::vec3 position;
		glm::quat rot;
		glm::vec3 scale;
	};

	static DecomposedMatrix decomposeMtx(glm::mat4 const& m);

	// Sets the local matrix and position, scale and rotation based on the world matrix of the entity.
	void setLocalBasedOnWorld(entt::entity entity);

private:
	bool hasAncestorChanged(entt::entity entity);
	glm::mat4x4 calculateModelMatrix(entt::entity entity);
	glm::mat4x4 calculateLocalMatrix(entt::entity entity);

private:
	ECS& ecs;
};
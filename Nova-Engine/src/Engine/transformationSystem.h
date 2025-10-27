#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
class ECS;
struct Transform;
struct EntityData;

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

	// Sets the world matrix and position, scale and rotation based on the local matrix of the entity.
	// just a more convenient public facing function that doesnt require additional parameters.
	void setLocalTransformFromWorld(entt::entity entity);



private:
	// Sets the local matrix and position, scale and rotation based on the world matrix of the entity.
	void setLocalTransformFromWorld(Transform& transform, EntityData& entityData);

	void setChildrenDirtyFlag(entt::entity entity);

	// Gets the most updated model matrix of a given entity.
	// Recalculates the model matrix if required.
	glm::mat4x4 const& getUpdatedModelMatrix(entt::entity entity);
	
	// Recalculate model matrix due to changes in ancestor's world transform.
	void recalculateModelMatrix(entt::entity entity);

private:
	entt::registry& registry;
	entt::dispatcher& eventDispatcher;
};
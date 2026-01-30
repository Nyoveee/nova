#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "export.h"

#include <entt/entt.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class ECS;
class Engine;
struct Transform;
struct EntityData;

class TransformationSystem {
public:
	TransformationSystem(Engine& engine, ECS& ecs);

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

	// Recalculate model matrix due to changes in ancestor's world transform.
	ENGINE_DLL_API void recalculateModelMatrix(entt::entity entity);
	ENGINE_DLL_API void updateWorldMatrix(Transform& transform);
	ENGINE_DLL_API void updateLocalMatrix(Transform& transform);

	void setChildrenDirtyFlag(entt::entity entity);

private:
	// Sets the local matrix and position, scale and rotation based on the world matrix of the entity.
	void setLocalTransformFromWorld(Transform& transform, EntityData& entityData);

	void setSocketDirtyFlag(entt::entity entity);

	// Gets the most updated model matrix of a given entity.
	// Recalculates the model matrix if required.
	glm::mat4x4 const& getUpdatedModelMatrix(entt::entity entity);

private:
	Engine& engine;
	entt::registry& registry;
	entt::dispatcher& eventDispatcher;
};
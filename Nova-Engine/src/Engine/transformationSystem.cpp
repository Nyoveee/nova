#include <iostream>

#include "transformationSystem.h"
#include "ecs.h"

#include "Component/component.h"

// Reference: https://stackoverflow.com/questions/17918033/glm-decompose-mat4-into-translation-and-rotation
TransformationSystem::DecomposedMatrix TransformationSystem::decomposeMtx(glm::mat4 const& m) {
	glm::vec3 pos = m[3];
	glm::vec3 scale;

	for (int i = 0; i < 3; i++)
		scale[i] = glm::length(glm::vec3(m[i]));

	const glm::mat3 rotMtx(
		glm::vec3(m[0]) / scale[0],
		glm::vec3(m[1]) / scale[1],
		glm::vec3(m[2]) / scale[2]
	);
	
	glm::quat rot = glm::quat_cast(rotMtx);

	return {
		pos,
		rot,
		scale
	};
}

TransformationSystem::TransformationSystem(ECS& ecs) :
	ecs {ecs}
{}

void TransformationSystem::update() {
	entt::registry& registry = ecs.registry;

	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		//// Figure out if the entity requires updating it's model view matrix.
		//if (!transform.recentlyUpdated) {
		//	if (
		//			transform.position	!= transform.lastPosition
		//		||	transform.scale		!= transform.lastScale
		//		||	transform.rotation	!= transform.lastRotation
		//	) {
		//		transform.recentlyUpdated = true;

		//		transform.lastPosition	= transform.position;
		//		transform.lastScale		= transform.scale;
		//		transform.lastRotation	= transform.rotation;
		//	}
		//}

		//// Calculate their model matrix, if there is a change in transform.
		//if (!transform.recentlyUpdated) {
		//	continue;
		//}

		if (entityData.parent == entt::null) {
			transform.modelMatrix = { 1 };
			transform.modelMatrix = glm::translate(transform.modelMatrix, transform.position);
			transform.modelMatrix = glm::scale(transform.modelMatrix, transform.scale);
		}
		else {
			transform.localMatrix = { 1 };
			transform.localMatrix = glm::translate(transform.localMatrix, transform.localPosition);
			transform.localMatrix = glm::scale(transform.localMatrix, transform.localScale);
		}
	}

	// Calculate the model matrix of all child objects.
	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		// We find out if itself or any ancestor has changed their transform.
		// If so, we update the child objects model matrix.

		// We ignore root game objects because they have no parents haha
		if (entityData.parent == entt::null) {
			transform.recentlyUpdated = false;
			continue;
		}

		//// Itself and ancestors has no changed
		//if (!hasAncestorChanged(entity)) {
		//	continue;
		//}

		// Recalculate model matrix if so..
		transform.modelMatrix = calculateModelMatrix(entity);

		// Set the appropriate new world position
		auto [position, rotation, scale] = TransformationSystem::decomposeMtx(transform.modelMatrix);
		transform.position = position;
		//transform.rotation = rotation;
		transform.scale = scale;
	}
}

bool TransformationSystem::hasAncestorChanged(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	bool hasChanged = registry.get<Transform>(entity).recentlyUpdated;

	if (hasChanged) {
		return true;
	}

	EntityData& entityData = registry.get<EntityData>(entity);
	
	if (entityData.parent != entt::null) {
		hasChanged |= hasAncestorChanged(entityData.parent);
	}

	return hasChanged;
}

glm::mat4x4 TransformationSystem::calculateModelMatrix(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	EntityData& entityData = registry.get<EntityData>(entity);
	Transform& transform = registry.get<Transform>(entity);

	glm::mat4x4 modelMatrix;
	if (entityData.parent != entt::null) {
		modelMatrix = calculateModelMatrix(entityData.parent) * transform.localMatrix;
	}
	else {
		modelMatrix = transform.modelMatrix;
	}

	return modelMatrix;
}

glm::mat4x4 TransformationSystem::calculateLocalMatrix(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	EntityData& entityData = registry.get<EntityData>(entity);
	Transform& transform = registry.get<Transform>(entity);

	if (entityData.parent == entt::null) {
		return transform.modelMatrix;
	}

	Transform& parentTransform = registry.get<Transform>(entityData.parent);
	glm::mat4 inverseParentWorldMatrix = glm::inverse(parentTransform.modelMatrix);
	return inverseParentWorldMatrix * transform.modelMatrix;
}

void TransformationSystem::setLocalBasedOnWorld(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	Transform& transform = registry.get<Transform>(entity);

	transform.localMatrix = calculateLocalMatrix(entity);

	auto [localPosition, localRotation, localScale] = TransformationSystem::decomposeMtx(transform.localMatrix);
	transform.localPosition = localPosition;
	transform.localScale = localScale;
	//transform.localRotation = localRotation;
}

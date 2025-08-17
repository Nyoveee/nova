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
		// Figure out if the entity requires updating it's world matrix.
		if (
				transform.position	!= transform.lastPosition
			||	transform.scale		!= transform.lastScale
			||	transform.rotation	!= transform.lastRotation
		) {
			transform.lastPosition	= transform.position;
			transform.lastScale		= transform.scale;
			transform.lastRotation	= transform.rotation;

			// Let's update the world matrix.
			transform.worldHasChanged = true;

			transform.modelMatrix = { 1 };
			transform.modelMatrix = glm::translate(transform.modelMatrix, transform.position);
			transform.modelMatrix = glm::scale(transform.modelMatrix, transform.scale);
		}

		// Figure out if the entity requires updating it's local matrix.
		// Root entities have no use for local transforms.
		if (entityData.parent == entt::null) {
			continue;
		}
		
		// We ignore local transformation changes if there is already a world change.
		// Our world matrix has already been calculated.
		if (transform.worldHasChanged) {
			continue;
		}

		if (
				transform.localPosition	!= transform.lastLocalPosition
			||	transform.localScale	!= transform.lastLocalScale
			||	transform.localRotation	!= transform.lastLocalRotation
		) {
			transform.localHasChanged = true;

			// We recalculate local matrix if there is a change in local transform.
			transform.lastLocalPosition	= transform.localPosition;
			transform.lastLocalScale	= transform.localScale;
			transform.lastLocalRotation	= transform.localRotation;

			transform.localMatrix = { 1 };
			transform.localMatrix = glm::translate(transform.localMatrix, transform.localPosition);
			transform.localMatrix = glm::scale(transform.localMatrix, transform.localScale);
		}
	}

	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		// Root entities do not need to worry about hirerarchy.
		if (entityData.parent == entt::null) {
			goto endOfLoop;
		}

		if (!transform.worldHasChanged) {
			// World transform did not change. This means it has not been directly edited.
			// In that case we want to change world transform if any of the ancestors has been transformed 
			// (regardless local or world)
			// This creates the hierarchy effect.

			if (!hasAncestorChanged(entity)) {
				// No transform changes in the hirerarchy.
				continue;
			}

			glm::mat4x4 parentWorldMatrix = registry.get<Transform>(entityData.parent).modelMatrix;
			transform.modelMatrix = parentWorldMatrix * transform.localMatrix;

			// Let's set the appropriate new world transforms via matrix decomposition.
			auto [position, rotation, scale] = TransformationSystem::decomposeMtx(transform.modelMatrix);
			transform.position = position;
			transform.scale = scale;
			
			transform.lastPosition = position;
			transform.lastScale = scale;
		}
		else {
			// World transform has changed. This means it has been directly edited.
			// In this case we want to calculate the appropriate local transform corresponding to this world transform.
			// This preserves the hierarchy effect for the future.

			glm::mat4x4 parentInverseWorld = glm::inverse(registry.get<Transform>(entityData.parent).modelMatrix);
			transform.localMatrix = parentInverseWorld * transform.modelMatrix;

			// Let's set the appropriate new local transforms via matrix decomposition.
			auto [localPosition, localRotation, localScale] = TransformationSystem::decomposeMtx(transform.localMatrix);
			transform.localPosition = localPosition;
			transform.localScale = localScale;

			transform.lastLocalPosition = localPosition;
			transform.lastLocalScale = localScale;
		}

	endOfLoop:
		transform.worldHasChanged = false;
		transform.localHasChanged = false;
	}
}

bool TransformationSystem::hasAncestorChanged(entt::entity entity) {
	entt::registry& registry = ecs.registry;
	bool hasChanged = registry.get<Transform>(entity).localHasChanged || registry.get<Transform>(entity).worldHasChanged;

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

#include <iostream>

#include "transformationSystem.h"
#include "ecs.h"

#include "Component/component.h"
#include "nova_math.h"
#include "Profiling.h"

TransformationSystem::TransformationSystem(ECS& ecs) :
	registry {ecs.registry}
{}

void TransformationSystem::update() {
	ZoneScoped;
	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		// Figure out if the entity requires updating it's world matrix due to world transform change.
		if (
				transform.worldHasChanged
			||	transform.position		!= transform.lastPosition
			||	transform.scale			!= transform.lastScale
			||	transform.eulerAngles	!= transform.lastEulerAngles
			||	!glm::all(glm::epsilonEqual(transform.rotation, transform.lastRotation, 1e-4f))
		) {
			// Let's update the world matrix.
			transform.worldHasChanged = true;
			
			// Quartenions changed, let's update our euler angles.
			if (!glm::all(glm::epsilonEqual(transform.rotation, transform.lastRotation, 1e-4f))) {
				transform.rotation = glm::normalize(transform.rotation);
				transform.eulerAngles = transform.rotation;
			}
			// Euler angles changed, let's update our quartenions.
			else if (transform.eulerAngles != transform.lastEulerAngles) {
				transform.rotation = transform.eulerAngles;
				transform.rotation = glm::normalize(transform.rotation);
			}

			transform.lastPosition		= transform.position;
			transform.lastScale			= transform.scale;
			transform.lastRotation		= transform.rotation;
			transform.lastEulerAngles	= transform.eulerAngles;

			transform.modelMatrix = { 1 };
			transform.modelMatrix = glm::translate(transform.modelMatrix, transform.position);
			transform.modelMatrix = transform.modelMatrix * glm::mat4_cast(transform.rotation);
			transform.modelMatrix = glm::scale(transform.modelMatrix, transform.scale);

			transform.normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform.modelMatrix)));

			// All childrens will have to reupdate their world transform (if it's not modified directly).
			setChildrenDirtyFlag(entity);
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
				transform.localPosition		!= transform.lastLocalPosition
			||	transform.localScale		!= transform.lastLocalScale
			||	transform.localEulerAngles	!= transform.lastLocalEulerAngles		// Euler angles not consistent with quartenions anymore.
			||	!glm::all(glm::epsilonEqual(transform.localRotation, transform.lastLocalRotation, 1e-4f))
		) {
			// World matrix needs recalculating because local matrix has been modified.
			transform.needsRecalculating = true;
			
			// All childrens will have to reupdate their world transform (if it's not modified directly).
			setChildrenDirtyFlag(entity);

			// Quartenions changed, let's update our euler angles.
			if (!glm::all(glm::epsilonEqual(transform.localRotation, transform.lastLocalRotation, 1e-4f))) {
				transform.localRotation = glm::normalize(transform.localRotation);
				transform.localEulerAngles = transform.localRotation;
			}
			// Euler angles changed, let's update our quartenions.
			else if (!glm::all(glm::epsilonEqual(transform.localRotation, glm::quat{ transform.localEulerAngles }, 1e-4f))) {
				transform.localRotation = transform.localEulerAngles;
				transform.localRotation = glm::normalize(transform.localRotation);
			}

			// We recalculate local matrix if there is a change in local transform.
			transform.lastLocalPosition		= transform.localPosition;
			transform.lastLocalScale		= transform.localScale;
			transform.lastLocalRotation		= transform.localRotation;
			transform.lastLocalEulerAngles	= transform.localEulerAngles;

			transform.localMatrix = { 1 };
			transform.localMatrix = glm::translate(transform.localMatrix, transform.localPosition);
			transform.localMatrix = transform.localMatrix * glm::mat4_cast(transform.localRotation);
			transform.localMatrix = glm::scale(transform.localMatrix, transform.localScale);
		}
	}

	// We will reupdate all world transforms that are affected indirectly due to ancenstor's change in world and local transform.
	// If it is also going to be affected indirectly we need to recalculate that world matrix.
	// If entity has already modified world transform, we ignore any indirect world transformations due to ancestor change.
	// At this point, we also know that all local matrixes are valid (for entities that did not directly edit their world transform).
	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		// Root entities do not need to worry about hirerarchy.
		if (entityData.parent == entt::null) {
			goto endOfLoop;
		}

		if (transform.worldHasChanged) {
			// World transform has changed. This means it has been directly edited.
			// If world transform is modified directly, we ignore all indirect modifications due to ancenstor's transform change.
			// In this case we want to calculate the appropriate local transform corresponding to this world transform.
			// This preserves the hierarchy effect for the future.
			setLocalTransformFromWorld(transform, entityData);
		}
		else {
			// World transform has not changed. This means it can be prone to indirect change due to ancestor's transform change.
			// In this case we want to calculate the new world transform due to any change in ancestor.
			
			// Either none of the ancestors has changed their transform or world transform has been recalculated.
			if (!transform.needsRecalculating) {
				continue;
			}

			// To recalculate our model matrix, we also need make sure the parent's world matrix is updated
			// We recursively check its parent's model matrix, until we know its updated or we reach a root entity.
			recalculateModelMatrix(entity);
			transform.needsRecalculating = false;
		}

	endOfLoop:
		transform.worldHasChanged = false;
	}
}

void TransformationSystem::setLocalTransformFromWorld(entt::entity entity) {
	Transform& transform = registry.get<Transform>(entity);
	EntityData& entityData = registry.get<EntityData>(entity);

	setLocalTransformFromWorld(transform, entityData);
}

void TransformationSystem::setLocalTransformFromWorld(Transform& transform, EntityData& entityData) {
	Transform& parentTransform = registry.get<Transform>(entityData.parent);
	glm::mat4 inverseParentWorldMatrix = glm::inverse(parentTransform.modelMatrix);
	transform.localMatrix = inverseParentWorldMatrix * transform.modelMatrix;

	auto [localPosition, localRotation, localScale] = Math::decomposeMatrix(transform.localMatrix);
	transform.localPosition = localPosition;
	transform.localScale = localScale;
	transform.localRotation = glm::normalize(localRotation);

	transform.lastLocalPosition = localPosition;
	transform.lastLocalScale = localScale;
	transform.lastLocalRotation = glm::normalize(localRotation);

	transform.localEulerAngles = transform.localRotation;
	transform.lastLocalEulerAngles = transform.localEulerAngles;
}

void TransformationSystem::setChildrenDirtyFlag(entt::entity entity) {
	EntityData& entityData = registry.get<EntityData>(entity);

	for (entt::entity child : entityData.children) {
		registry.get<Transform>(child).needsRecalculating = true;
		setChildrenDirtyFlag(child);
	}
}

glm::mat4x4 const& TransformationSystem::getUpdatedModelMatrix(entt::entity entity) {
	Transform& transform = registry.get<Transform>(entity);

	// Attempts to get the most updated model matrix.
	if (transform.needsRecalculating) {
		EntityData& entityData = registry.get<EntityData>(entity);
		transform.modelMatrix = getUpdatedModelMatrix(entityData.parent) * transform.localMatrix;
		transform.needsRecalculating = false;

		// Let's set the appropriate new world transforms via matrix decomposition.
		auto [position, rotation, scale] = Math::decomposeMatrix(transform.modelMatrix);
		transform.position = position;
		transform.scale = scale;
		transform.rotation = glm::normalize(rotation);

		transform.lastPosition = position;
		transform.lastScale = scale;
		transform.lastRotation = glm::normalize(rotation);

		transform.eulerAngles = transform.rotation;
		transform.lastEulerAngles = transform.eulerAngles;

		transform.normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform.modelMatrix)));
	}

	return transform.modelMatrix;
}

void TransformationSystem::recalculateModelMatrix(entt::entity entity) {
	getUpdatedModelMatrix(entity);
}

#include <iostream>

#include "transformationSystem.h"
#include "ECS/ECS.h"
#include "ECS/Events.h"

#include "component.h"
#include "nova_math.h"
#include "Profiling.h"

#include "Engine/engine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

constexpr glm::vec3 defaultFront = { 0.f, 0.f, 1.f };
constexpr glm::vec3 defaultUp = { 0, 1.f, 0 };
glm::vec3 defaultRight = { glm::normalize(glm::cross(defaultFront, defaultUp)) };

TransformationSystem::TransformationSystem(Engine& engine, ECS& ecs) :
	engine			{ engine },
	registry		{ ecs.registry },
	eventDispatcher	{ ecs.systemEventDispatcher }
{}

void TransformationSystem::update() {
#if !defined(NOVA_INSTALLER)
	ZoneScoped;
#endif

	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		// Figure out if the entity requires updating it's world matrix due to world transform change.
		if (
				transform.worldHasChanged
			||	transform.position			!= transform.lastPosition
			||	transform.scale				!= transform.lastScale
			||	!glm::all(glm::epsilonEqual(glm::vec3{ transform.eulerAngles }, glm::vec3{ transform.lastEulerAngles }, 1e-4f))
			||  !glm::all(glm::epsilonEqual(transform.rotation, transform.lastRotation, 1e-4f))
		) {
			// Let's update the world matrix.
			transform.worldHasChanged = true;

			// Quartenions changed, let's update our euler angles.
			if (!glm::all(glm::epsilonEqual(transform.rotation, transform.lastRotation, 1e-4f))) {
				transform.rotation = glm::normalize(transform.rotation);
				transform.eulerAngles = transform.rotation;
			}
			// Euler angles changed, let's update our quartenions.
			else if (!glm::all(glm::epsilonEqual(glm::vec3{ transform.eulerAngles }, glm::vec3{ transform.lastEulerAngles }, 1e-4f))) {
				transform.rotation = transform.eulerAngles;
				transform.rotation = glm::normalize(transform.rotation);
			}

			// Update events, try to save old data for more tweking
			eventDispatcher.trigger<TransformUpdateEvent>(TransformUpdateEvent{ entity, transform.lastPosition, transform.lastScale, transform.lastRotation });

			if (transform.worldModifiedByScripting) {
				transform.deltaPosition = transform.position - transform.lastPosition;
				transform.deltaScale = transform.scale - transform.lastScale;
				transform.deltaRotation = transform.rotation - transform.lastRotation;
			}

			updateWorldMatrix(transform);

			// All childrens will have to reupdate their world transform (if it's not modified directly).
			setChildrenDirtyFlag(entity);
		}

		// Figure out if the entity requires updating it's local matrix.
		if (
				transform.localPosition != transform.lastLocalPosition
			||	transform.localScale != transform.lastLocalScale
			||	!glm::all(glm::epsilonEqual(glm::vec3{ transform.localEulerAngles }, glm::vec3{ transform.lastLocalEulerAngles }, 1e-4f))		// Euler angles not consistent with quartenions anymore.
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
			else if (!glm::all(glm::epsilonEqual(glm::vec3{ transform.localEulerAngles }, glm::vec3{ transform.lastLocalEulerAngles }, 1e-4f))) {
				transform.localRotation = transform.localEulerAngles;
				transform.localRotation = glm::normalize(transform.localRotation);
			}

			//Update events, try to save old data for more tweking
			eventDispatcher.trigger<TransformUpdateEvent>(TransformUpdateEvent{ entity, transform.lastLocalPosition, transform.lastLocalScale, transform.lastLocalRotation });

			// We recalculate local matrix if there is a change in local transform.
			updateLocalMatrix(transform);
		}
	}

	// We will reupdate all world transforms that are affected indirectly due to ancenstor's change in world and local transform.
	// If it is also going to be affected indirectly we need to recalculate that world matrix.
	// If entity has already modified world transform, we ignore any indirect world transformations due to ancestor change.
	// At this point, we also know that all local matrixes are valid (for entities that did not directly edit their world transform).
	for (auto&& [entity, entityData, transform] : registry.view<EntityData, Transform>().each()) {
		if (transform.needsRecalculating) {
			// World transform has not changed. This means it can be prone to indirect change due to ancestor's transform change.
			// In this case we want to calculate the new world transform due to any change in ancestor.

			// Either none of the ancestors has changed their transform or world transform has been recalculated.

			// Update events, try to save old data for more tweking
			eventDispatcher.trigger<TransformUpdateEvent>(TransformUpdateEvent{ entity, transform.lastPosition, transform.lastScale, transform.lastRotation });

			// To recalculate our model matrix, we also need make sure the parent's world matrix is updated
			// We recursively check its parent's model matrix, until we know its updated or we reach a root entity.
			recalculateModelMatrix(entity);
		}
		
		if (transform.worldHasChanged) {
			// Update events, try to save old data for more tweking
			eventDispatcher.trigger<TransformUpdateEvent>(TransformUpdateEvent{ entity, transform.lastLocalPosition, transform.lastLocalScale, transform.lastLocalRotation });

			// World transform has changed. This means it has been directly edited.
			// If world transform is modified directly, we ignore all indirect modifications due to ancenstor's transform change.
			// In this case we want to calculate the appropriate local transform corresponding to this world transform.
			// This preserves the hierarchy effect for the future.
			setLocalTransformFromWorld(transform, entityData);
		}
	}
}


void TransformationSystem::setLocalTransformFromWorld(entt::entity entity) {
	Transform& transform = registry.get<Transform>(entity);
	EntityData& entityData = registry.get<EntityData>(entity);

	setLocalTransformFromWorld(transform, entityData);
}

void TransformationSystem::setLocalTransformFromWorld(Transform& transform, EntityData& entityData) {
	if (entityData.parent == entt::null) {
		transform.localMatrix = transform.modelMatrix;
	}
	else {
		Transform& parentTransform = registry.get<Transform>(entityData.parent);
		glm::mat4 inverseParentWorldMatrix = glm::inverse(parentTransform.modelMatrix);
		transform.localMatrix = inverseParentWorldMatrix * transform.modelMatrix;
	}

	auto [localPosition, localRotation, localScale] = Math::decomposeMatrix(transform.localMatrix);
	transform.localPosition = localPosition;
	transform.localScale = localScale;
	transform.localRotation = glm::normalize(localRotation);

	updateLocalMatrix(transform);
	transform.worldHasChanged = false;
}

void TransformationSystem::setChildrenDirtyFlag(entt::entity entity) {
	EntityData& entityData = registry.get<EntityData>(entity);

	for (entt::entity child : entityData.children) {
		registry.get<Transform>(child).needsRecalculating = true;
		setChildrenDirtyFlag(child);
	}
}

void TransformationSystem::setSocketDirtyFlag(entt::entity entity)
{
	SkinnedMeshRenderer* mesh = registry.try_get<SkinnedMeshRenderer>(entity);
	if (!mesh)
		return;

	for (auto& socketConnection : mesh->socketConnections) {
		entt::entity connectedEntity = socketConnection.second;
		Transform* trans = registry.try_get<Transform>(connectedEntity);
		if (trans) {
			trans->needsRecalculating = true;
			setChildrenDirtyFlag(connectedEntity);
		}
	}
}

glm::mat4x4 const& TransformationSystem::getUpdatedModelMatrix(entt::entity entity) {
	Transform& transform = registry.get<Transform>(entity);
	EntityData& entityData = registry.get<EntityData>(entity);

	// Attempts to get the most updated model matrix.
	if (transform.needsRecalculating) {
		if (entityData.parent == entt::null) {
			// parent to nothing..
			transform.modelMatrix = transform.localMatrix;
		}
		else {
			// let's find the bone of this parent if its attached to it's socket
			SkinnedMeshRenderer const* skinnedMeshRenderer = registry.try_get<SkinnedMeshRenderer>(entityData.parent);

			glm::mat4 boneWorldMatrix{ 1.f };

			if (engine.isInSimulationMode() && skinnedMeshRenderer && entityData.attachedSocket < skinnedMeshRenderer->bonesFinalMatrices[skinnedMeshRenderer->currentBoneMatrixIndex].size()) {
				auto&& [model, _] = engine.resourceManager.getResource<Model>(skinnedMeshRenderer->modelId);
				
				if (model) {
					auto boneFinalMatrix = skinnedMeshRenderer->bonesFinalMatrices[skinnedMeshRenderer->currentBoneMatrixIndex][entityData.attachedSocket];
					auto&& [position, rotation, __] = Math::decomposeMatrix(boneFinalMatrix);
					boneWorldMatrix = glm::translate(boneWorldMatrix, position * model->scale);	// i dont actually wanna scale the mode, just the offset from local space..
					boneWorldMatrix = boneWorldMatrix * glm::mat4_cast(rotation);
				}
			} 
				
			transform.modelMatrix = (getUpdatedModelMatrix(entityData.parent) * boneWorldMatrix) * transform.localMatrix;
		}

		transform.needsRecalculating = false;

		// Let's set the appropriate new world transforms via matrix decomposition.
		auto [position, rotation, scale] = Math::decomposeMatrix(transform.modelMatrix);

		transform.position = position;
		transform.scale = scale;
		transform.rotation = glm::normalize(rotation);

		if (transform.worldModifiedByScripting) {
			transform.worldModifiedByScripting = false;

			transform.position += transform.deltaPosition;
			transform.scale += transform.deltaScale;
			transform.rotation += transform.deltaRotation;
			transform.rotation = glm::normalize(transform.rotation);
		}

		updateWorldMatrix(transform);
	}

	return transform.modelMatrix;
}

void TransformationSystem::recalculateModelMatrix(entt::entity entity) {
	getUpdatedModelMatrix(entity);
}

void TransformationSystem::updateWorldMatrix(Transform& transform) {
	transform.front = transform.rotation * defaultFront;
	transform.up = transform.rotation * defaultUp;
	transform.right = transform.rotation * defaultRight;
	transform.lastPosition = transform.position;
	transform.lastScale = transform.scale;
	transform.lastRotation = transform.rotation;
	transform.lastEulerAngles = transform.eulerAngles;

	transform.modelMatrix = { 1 };
	transform.modelMatrix = glm::translate(transform.modelMatrix, transform.position);
	transform.modelMatrix = transform.modelMatrix * glm::mat4_cast(transform.rotation);
	transform.modelMatrix = glm::scale(transform.modelMatrix, transform.scale);

	transform.normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform.modelMatrix)));
}

void TransformationSystem::updateLocalMatrix(Transform& transform) {
	transform.lastLocalPosition = transform.localPosition;
	transform.lastLocalScale = transform.localScale;
	transform.lastLocalRotation = transform.localRotation;
	transform.lastLocalEulerAngles = transform.localEulerAngles;

	transform.localMatrix = { 1 };
	transform.localMatrix = glm::translate(transform.localMatrix, transform.localPosition);
	transform.localMatrix = transform.localMatrix * glm::mat4_cast(transform.localRotation);
	transform.localMatrix = glm::scale(transform.localMatrix, transform.localScale);
}

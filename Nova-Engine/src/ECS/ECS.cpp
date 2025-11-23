#include <ranges>
#include <iostream>

#include "Engine/engine.h"
#include "ECS/ECS.h"
#include "component.h"
#include "Logger.h"

ECS::ECS(Engine& engine) : 
	registry		{}, 
	engine			{ engine },
	sceneManager	{ *this, engine ,engine.resourceManager },
	prefabManager	{engine}
{}

ECS::~ECS() {}

void ECS::setEntityParent(entt::entity childEntity, entt::entity newParentEntity, bool recalculateLocalTransform) {
	EntityData& childEntityData = registry.get<EntityData>(childEntity);
	EntityData& newParentEntityData = registry.get<EntityData>(newParentEntity);

	if (childEntity == newParentEntity) {
		return;
	}

	if (newParentEntity == childEntityData.parent) {
		return;
	}

	// This removes parent child relationship with the entity.
	if (newParentEntity == entt::null) {
		removeEntityParent(childEntity);
		return;
	}

	// 1. We need to check for potential cycle.
	// A cycle happens when if the childEntity's descendant contains newParentEntity.
	if (isDescendantOf(newParentEntity, childEntity)) {
		Logger::error("Cyclic relationship detected. Failed to set {} as {}'s new parent.", newParentEntityData.name, childEntityData.name);
		return;
	}

	// 2. Remove itself from the childrens of the old parent.
	if (childEntityData.parent != entt::null) {
		EntityData& oldParentEntityData = registry.get<EntityData>(childEntityData.parent);
		auto iterator = std::ranges::find(oldParentEntityData.children, childEntity);

		if (iterator == std::end(oldParentEntityData.children)) {
			Logger::error("This really shouldn't happen.. The invariant of parent child relationship has been broken.");
		}
		else {
			oldParentEntityData.children.erase(iterator);
		}
	}

	// 3. Add itself to the new parent.
	childEntityData.parent = newParentEntity;
	newParentEntityData.children.push_back(childEntity);

	// 4. Properly set the local transform of the entity.
	if(recalculateLocalTransform) 
		engine.transformationSystem.setLocalTransformFromWorld(childEntity);
}

void ECS::removeEntityParent(entt::entity childEntity) {
	EntityData& childEntityData = registry.get<EntityData>(childEntity);

	if (childEntityData.parent == entt::null) {
		return;
	}

	EntityData& oldParentEntityData = registry.get<EntityData>(childEntityData.parent);
	auto iterator = std::ranges::find(oldParentEntityData.children, childEntity);

	if (iterator == std::end(oldParentEntityData.children)) {
		Logger::error("This really shouldn't happen.. The invariant of parent child relationship has been broken.");
	}
	else {
		oldParentEntityData.children.erase(iterator);
		childEntityData.parent = entt::null;
	}
}

// Finds out if a given entity is a descendant of parent (direct and indirect children).
bool ECS::isDescendantOf(entt::entity entity, entt::entity parent) {
	if (entity == parent) return false;

	EntityData& parentData = registry.get<EntityData>(parent);

	bool isDescendant = false;

	for (entt::entity child : parentData.children) {
		if (child == entity) {
			isDescendant = true;
			break;
		}
		else {
			isDescendant |= isDescendantOf(entity, child);
			if (isDescendant) break;
		}
	}

	return isDescendant;
}

void ECS::deleteEntity(entt::entity entity) {
	EntityData* entityData = registry.try_get<EntityData>(entity);
	
	if (entityData) {
		// =======================
		// 1. Update it's children' parent.
		// Set the children's new parent to the current's entity parent, if any.
		// =======================
		entt::entity parent = entityData->parent;

		for (entt::entity child : entityData->children) {
			EntityData& childEntityData = registry.get<EntityData>(child);
			childEntityData.parent = parent;
		}

		// =======================
		// 2. Update parent's children array.
		// =======================
		if (parent != entt::null) {
			// Remove this entity from list of children
			EntityData& parentEntityData = registry.get<EntityData>(parent);

			auto iterator = std::ranges::find(parentEntityData.children, entity);
			assert(iterator != parentEntityData.children.end() && "Invariant broken.");
			parentEntityData.children.erase(iterator);

			// Inherit grandchildren as the new children.
			for (entt::entity child : entityData->children) {
				parentEntityData.children.push_back(child);
			}
		}
	}

	// 3. Delete the entity!
	registry.destroy(entity);
}

void ECS::setActive(entt::entity entity, bool isActive) {
	EntityData& entityData = registry.get<EntityData>(entity);
	
	// there is a change in active status..
	if (entityData.isActive != isActive) {
		entityData.isActive = isActive;

		// destruction / construction of physics body when enabling or disabling..
		if (isActive) {
			engine.physicsManager.addBodiesToSystem(registry, entity);
			engine.navigationSystem.SetAgentActive(entity);
		
		}
		else {
			engine.physicsManager.removeBodiesFromSystem(registry, entity);
			engine.navigationSystem.SetAgentInactive(entity);
		}
	}

	for (entt::entity child : entityData.children) {
		setActive(child, isActive);
	}
}

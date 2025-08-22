#include <spdlog/spdlog.h>

#include <ranges>
#include <iostream>

#include "engine.h"
#include "ECS.h"
#include "Component/component.h"

ECS::ECS(Engine& engine) : 
	registry	{}, 
	engine		{ engine } 
{}

ECS::~ECS() {}

void ECS::setEntityParent(entt::entity childEntity, entt::entity newParentEntity) {
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
		spdlog::error("Cyclic relationship detected. Failed to set {} as {}'s new parent.", newParentEntityData.name, childEntityData.name);
		return;
	}

	// 2. Remove itself from the childrens of the old parent.
	if (childEntityData.parent != entt::null) {
		EntityData& oldParentEntityData = registry.get<EntityData>(childEntityData.parent);
		auto iterator = std::ranges::find(oldParentEntityData.children, childEntity);

		if (iterator == std::end(oldParentEntityData.children)) {
			spdlog::error("This really shouldn't happen.. The invariant of parent child relationship has been broken.");
		}
		else {
			oldParentEntityData.children.erase(iterator);
		}
	}

	// 3. Add itself to the new parent.
	childEntityData.parent = newParentEntity;
	newParentEntityData.children.push_back(childEntity);

	// 4. Properly set the local transform of the entity.
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
		spdlog::error("This really shouldn't happen.. The invariant of parent child relationship has been broken.");
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

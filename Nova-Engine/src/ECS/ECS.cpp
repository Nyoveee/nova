#include <ranges>
#include <iostream>

#include "ECS/ECS.h"
#include "component.h"
#include "Logger.h"
#include "Engine/engine.h"

ECS::ECS(Engine& engine) : 
	registry		{}, 
	engine			{ engine },
	sceneManager	{ *this, engine, engine.resourceManager },
	prefabManager	{ engine },
	canvasUi		{ entt::null }
{}

ECS::~ECS() {
#if false
	registry.on_construct<Canvas>().connect<&ECS::onCanvasCreation>(*this);
	registry.on_destroy<Canvas>().connect<&ECS::onCanvasDestruction>(*this);
#endif
}

void ECS::setEntityParent(entt::entity childEntity, entt::entity newParentEntity, bool recalculateLocalTransform) {
	setEntityParent(childEntity, newParentEntity, recalculateLocalTransform, registry);
}

void ECS::setEntityParent(entt::entity childEntity, entt::entity newParentEntity, bool recalculateLocalTransform, entt::registry& p_registry) {
	EntityData& childEntityData = p_registry.get<EntityData>(childEntity);
	EntityData& newParentEntityData = p_registry.get<EntityData>(newParentEntity);

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
		EntityData& oldParentEntityData = p_registry.get<EntityData>(childEntityData.parent);
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
		for (entt::entity child : entityData->children) {
			// a separate function is used to recursively delete children, because no invariant needs to be maintained.
			deleteEntityRecursively(child);
		}

		// =======================
		// Update parent's children array.
		// =======================
		if (entityData->parent != entt::null) {
			// Remove this entity from list of children
			EntityData& parentEntityData = registry.get<EntityData>(entityData->parent);

			auto iterator = std::ranges::find(parentEntityData.children, entity);
			assert(iterator != parentEntityData.children.end() && "Invariant broken.");
			parentEntityData.children.erase(iterator);
		}
	}

	// Delete the entity!
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

bool ECS::isParentCanvas(entt::entity entity) {
	if (entity == entt::null) {
		return false;
	}

	if (registry.any_of<Canvas>(entity)) {
		return true;
	}

	EntityData& entityData = registry.get<EntityData>(entity);
	return isParentCanvas(entityData.parent);
}

void ECS::recordOriginalScene() {
	originalScene = sceneManager.getCurrentScene();
}

void ECS::restoreOriginalScene() {
	sceneManager.loadScene(originalScene);
}

bool ECS::isComponentActive(entt::entity entity, ComponentID componentID)
{
	EntityData& entityData{ registry.get<EntityData>(entity) };
	return !entityData.inactiveComponents.count(componentID);
}

void ECS::setComponentActive(entt::entity entity, ComponentID componentID, bool isActive)
{
	EntityData& entityData{ registry.get<EntityData>(entity) };
	std::unordered_set<ComponentID>& inactiveComponents{ entityData.inactiveComponents };
	if (isActive && inactiveComponents.count(componentID)) {
		inactiveComponents.erase(std::find(std::begin(inactiveComponents), std::end(inactiveComponents), componentID));
		if (componentID == typeid(NavMeshAgent).hash_code())
			engine.navigationSystem.SetAgentActive(entity);
		if (componentID == typeid(Rigidbody).hash_code())
			engine.physicsManager.addBodiesToSystem(engine.ecs.registry, entity);
	}
	else if (!isActive && !inactiveComponents.count(componentID)) {
		inactiveComponents.insert(componentID);
		if (componentID == typeid(NavMeshAgent).hash_code())
			engine.navigationSystem.SetAgentInactive(entity);
		if (componentID == typeid(Rigidbody).hash_code())
			engine.physicsManager.removeBodiesFromSystem(engine.ecs.registry, entity);
	}
}

void ECS::deleteEntityRecursively(entt::entity entity) {
	EntityData* entityData = registry.try_get<EntityData>(entity);

	if (entityData) {
		for (entt::entity child : entityData->children) {
			deleteEntityRecursively(child);
		}
	}

	registry.destroy(entity);
}

#if false
void ECS::onCanvasCreation(entt::registry&, entt::entity entityID) {
	if (canvasUi != entt::null) {
		Logger::error("Scene already contains an entity with the Canvas component! Removing another canvas component from entity {}", static_cast<unsigned>(entityID));
		registry.remove<Canvas>(entityID);
		return;
	}

	canvasUi = entityID;
}

void ECS::onCanvasDestruction(entt::registry&, entt::entity entityID) {
	if (canvasUi != entityID) {
		return;
	}
	
	canvasUi = entt::null;
}
#endif
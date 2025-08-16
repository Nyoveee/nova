#pragma once

#include "export.h"
#include <entt/entt.hpp>

// A singleton ECS wrapper around the entt framework for ease of access from other classes.
class ECS {
public:
	DLL_API ECS();

	DLL_API ~ECS();
	DLL_API ECS(ECS const& other)				= delete;
	DLL_API ECS(ECS&& other)					= delete;
	DLL_API ECS& operator=(ECS const& other)	= delete;
	DLL_API ECS& operator=(ECS&& other)			= delete;

public:
	// Set newParentEntity as the new parent for childEntity.
	// You can pass entt::null as the new parent and this makes the child entity a root entity with no parent.
	DLL_API void setEntityParent(entt::entity childEntity, entt::entity newParentEntity);
	DLL_API void removeEntityParent(entt::entity childEntity);

	// Finds out if a given entity is a descendant of parent (direct and indirect children).
	DLL_API bool isDescendantOf(entt::entity entity, entt::entity parent);
public:
	// public!
	entt::registry registry;
};
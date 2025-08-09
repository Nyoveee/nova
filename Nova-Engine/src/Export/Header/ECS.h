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
	// public!
	entt::registry registry;
};
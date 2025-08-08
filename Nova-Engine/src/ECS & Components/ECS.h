#pragma once

#include <entt/entt.hpp>

// A singleton ECS wrapper around the entt framework for ease of access from other classes.
class ECS {
	ECS();
public:
	static ECS& instance();

	~ECS();
	ECS(ECS const& other)				= delete;
	ECS(ECS&& other)					= delete;
	ECS& operator=(ECS const& other)	= delete;
	ECS& operator=(ECS&& other)			= delete;

public:
	// public!
	entt::registry registry;
};
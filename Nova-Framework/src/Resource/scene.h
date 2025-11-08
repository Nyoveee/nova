#pragma once

#include "resource.h"

#include <unordered_set>
#include <entt/entt.hpp>

struct Layer {
	std::string name;
	std::unordered_set<entt::entity> entities;

	REFLECTABLE(
		name
	)
};

class Scene : public Resource {
public:
	FRAMEWORK_DLL_API Scene(ResourceID id, ResourceFilePath resourceFilePath) : Resource{ id, std::move(resourceFilePath) } {};

	FRAMEWORK_DLL_API ~Scene()								= default;
	FRAMEWORK_DLL_API Scene(Scene const& other)				= delete;
	FRAMEWORK_DLL_API Scene(Scene&& other)					= default;
	FRAMEWORK_DLL_API Scene& operator=(Scene const& other)	= delete;
	FRAMEWORK_DLL_API Scene& operator=(Scene&& other)		= default;
};
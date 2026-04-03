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

struct SceneProperties {
	NormalizedFloat brightness = 0.5f;
	NormalizedFloat contrast = 0.5f;
	NormalizedFloat saturation = 0.5f;

	NormalizedFloat temperature = 0.5f;
	NormalizedFloat tint = 0.5f;

	NormalizedFloat iblDiffuseStrength = 1.f;
	NormalizedFloat iblSpecularStrength = 1.f;

	REFLECTABLE(
		iblDiffuseStrength,
		iblSpecularStrength,
		brightness,
		contrast,
		saturation,
		temperature,
		tint
	)
};
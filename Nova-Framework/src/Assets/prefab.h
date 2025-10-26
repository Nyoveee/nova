#pragma once

#include "resource.h"

class Prefab : public Resource {
public:
	FRAMEWORK_DLL_API Prefab(ResourceID id, ResourceFilePath resourceFilePath) : Resource{ id, std::move(resourceFilePath) } {};

	FRAMEWORK_DLL_API ~Prefab() = default;
	FRAMEWORK_DLL_API Prefab(Prefab const& other) = delete;
	FRAMEWORK_DLL_API Prefab(Prefab&& other) = default;
	FRAMEWORK_DLL_API Prefab& operator=(Prefab const& other) = delete;
	FRAMEWORK_DLL_API Prefab& operator=(Prefab&& other) = default;
};
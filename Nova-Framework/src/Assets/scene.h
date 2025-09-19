#pragma once

#include "resource.h"

class Scene : public Resource {
public:
	FRAMEWORK_DLL_API Scene(ResourceID id) : Resource{ id } {};

	FRAMEWORK_DLL_API ~Scene()								= default;
	FRAMEWORK_DLL_API Scene(Scene const& other)				= delete;
	FRAMEWORK_DLL_API Scene(Scene&& other)					= default;
	FRAMEWORK_DLL_API Scene& operator=(Scene const& other)	= delete;
	FRAMEWORK_DLL_API Scene& operator=(Scene&& other)		= default;
};
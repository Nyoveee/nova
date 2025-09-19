#pragma once

#include "asset.h"

class Scene : public Asset {
public:
	FRAMEWORK_DLL_API Scene(ResourceID id) : Asset{ id } {};

	FRAMEWORK_DLL_API ~Scene()								= default;
	FRAMEWORK_DLL_API Scene(Scene const& other)				= delete;
	FRAMEWORK_DLL_API Scene(Scene&& other)					= default;
	FRAMEWORK_DLL_API Scene& operator=(Scene const& other)	= delete;
	FRAMEWORK_DLL_API Scene& operator=(Scene&& other)		= default;
};
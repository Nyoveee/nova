#pragma once

#include <entt/entt.hpp>
#include <memory>

#include "type_alias.h"

//Foward Declare
class Engine;
class ResourceManager;

class NavigationSystem
{
public: 
	NavigationSystem(Engine& engine);

	~NavigationSystem();
	NavigationSystem(NavigationSystem const& other)				= delete;
	NavigationSystem(NavigationSystem && other)					= delete;
	NavigationSystem& operator=(NavigationSystem const& other)	= delete;
	NavigationSystem& operator=(NavigationSystem&& other)		= delete;

public:
	ENGINE_DLL_API void setNewNavMesh(ResourceID navMeshId);
	ENGINE_DLL_API ResourceID getNavMeshId() const;

private:
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	ResourceID navMeshId;
};



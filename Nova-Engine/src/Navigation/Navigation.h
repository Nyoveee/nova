#pragma once

#include <entt/entt.hpp>
#include <memory>

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


private:

	unsigned char* navData; //navmesh data in the form of a binary blob. Can be serialised or passed to detour for navigation.
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;


};



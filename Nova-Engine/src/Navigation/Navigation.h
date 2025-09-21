#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>

#include <memory>
#include <functional>

#include "export.h"
#include "Detour/Detour/DetourNavMesh.h"
#include "Detour/Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourCrowd/DetourCrowd.h"

//Foward Declare
class Engine;
class ResourceManager;


//struct dtNavMeshDeleter {
//	void operator()(dtNavMesh* ptr) const noexcept {
//		if (ptr) dtFreeNavMesh(ptr);
//	}
//};



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

	DLL_API dtNavMesh* RegisterNavigationMesh(std::string const& agentName, unsigned char* navData, int const& dataSize);

	DLL_API void Terminate();

private:

	//unsigned char* navData; //navmesh data in the form of a binary blob. Can be serialised or passed to detour for navigation.
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	//static constexpr auto dtNavMeshDeleter = +[](dtNavMesh* ptr)
	//{
	//	if (ptr) dtFreeNavMesh(ptr);

	//};


	using uniqueNavMeshPtr = std::unique_ptr<dtNavMesh, dtNavMeshDeleter>;

	//In class stuff
	std::unordered_map<std::string, uniqueNavMeshPtr> navMeshMap;

};



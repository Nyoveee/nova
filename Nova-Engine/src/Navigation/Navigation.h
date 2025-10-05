#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

#include "Detour/Detour/DetourNavMesh.h"
#include "Detour/Detour/DetourNavMeshQuery.h"
#include "Detour/DetourCrowd/DetourCrowd.h"
#include "navMesh.h"


#include "type_alias.h"

//Foward Declare
class Engine;
class ResourceManager;


struct dtCrowdDeleter {
	void operator()(dtCrowd* ptr) const noexcept {
		if (ptr) dtFreeCrowd(ptr);
	}
};

struct dtQueryDeleter {
	void operator()(dtNavMeshQuery* ptr) const noexcept {
		if (ptr) dtFreeNavMeshQuery(ptr);
	}
};


class NavigationSystem
{
public: 
	NavigationSystem(Engine& engine);

	~NavigationSystem();
	NavigationSystem(NavigationSystem const& other)				= delete;
	NavigationSystem(NavigationSystem && other)					= delete;
	NavigationSystem& operator=(NavigationSystem const& other)	= delete;
	NavigationSystem& operator=(NavigationSystem&& other)		= delete;
	void update(float const& dt);

public:
	ENGINE_DLL_API void setNewNavMesh(ResourceID navMeshId);
	ENGINE_DLL_API ResourceID getNavMeshId() const;
	ENGINE_DLL_API void		  initNavMeshSystems();
	ENGINE_DLL_API void		  NavigationDebug();


//--------------------For C# scripting API-------------------------------------------------------------//
public:
	//Start navigation for a particular agent. Returns bool when unable to set destination to targetPosition.
	ENGINE_DLL_API bool setDestination(entt::entity entityID, glm::vec3 targetPosition );



private:
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	
	std::unordered_map<std::string, TypedResourceID<NavMesh> > sceneNavMeshes;

	std::unordered_map<std::string, std::unique_ptr<dtNavMeshQuery, dtQueryDeleter> > queryManager;

	std::unordered_map<std::string, std::unique_ptr<dtCrowd,dtCrowdDeleter> > crowdManager;

	ResourceID navMeshId;
};



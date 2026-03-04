#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
#include <memory>
#include "component.h"
#include <unordered_map>

#include "Detour/Detour/DetourNavMesh.h"
#include "Detour/Detour/DetourNavMeshQuery.h"
#include "Detour/DetourCrowd/DetourCrowd.h"
#include "AdditionalNavigationTypes.h"
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

struct navMeshOfflinkData;

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
	ENGINE_DLL_API void		  unloadNavMeshSystems();
	ENGINE_DLL_API void		  NavigationDebug();
	ENGINE_DLL_API void		  AddAgentsToSystem(entt::registry&, entt::entity entityID);
	ENGINE_DLL_API void		  RemoveAgentsFromSystem(entt::registry&, entt::entity entityID);
	ENGINE_DLL_API void		  InstantiateAgentsToSystem(entt::entity entityID, Transform const*const enttTransform, NavMeshAgent *const navMeshAgent);
	ENGINE_DLL_API void		  SetAgentActive(entt::entity entityID);
	ENGINE_DLL_API void		  SetAgentInactive(entt::entity entityID);
	ENGINE_DLL_API void		  SetAgentAutoOffMeshTraversalParams(NavMeshAgent& navMeshAgent, bool state);

//--------------------For C# scripting API-------------------------------------------------------------//
public:
	//--------- IN Script Library -----//
	//Start navigation for a particular agent. Returns bool false when unable to set destination to targetPosition.
	ENGINE_DLL_API bool setDestination(entt::entity entityID, glm::vec3 targetPosition );

	//returns a closest navmesh position from a center position. searchExtent is the search box given in dimensons X,Y,Z
	ENGINE_DLL_API std::optional<glm::vec3> SampleNavMeshPosition(std::string agentMeshName, glm::vec3 sourcePosition, glm::vec3 searchExtent);


	//Given two points get a list of waypoints from start to end if possible. Returns empty vector if no path found.
	// 
	// Currently if path is partial it will still return best guess path. check list[index-1] to see if it reached
	// destination.
	// Note: currently this function is set to find path lesser than 255 polys apart. if it fails because of it consider increasing arr size
	//Suggest to use Sample NavMeshPosition to ensure start and position are on valid navmesh. 
	ENGINE_DLL_API std::vector<glm::vec3> FindPath(std::string agentMeshName, glm::vec3 startPosition, glm::vec3 endPosition);

	//--------- IN Managed Types -----//

	//Warp the agent to this point, moves the transform as well, so it will turn
	ENGINE_DLL_API bool warp(NavMeshAgent& navMeshAgent,  glm::vec3 targetPosition);

	//return navMeshOfflinkData.valid == false if navMeshOffLink does not exist, rmb to check if navmeshofflink is taken. isOnOffMeshLinks() on C# side
	ENGINE_DLL_API navMeshOfflinkData getNavmeshOfflinkData(NavMeshAgent& navMeshAgent);

	ENGINE_DLL_API void CompleteOffLinkData(NavMeshAgent& navMeshAgent);

	//ENGINE_DLL_API void setAgentInactive(entt::entity entityID);

	//ENGINE_DLL_API void setAgentActive(entt::entity entityID);

	ENGINE_DLL_API void stopAgent(entt::entity entityID);

//--------------------Helper functions-------------------------------------------------------------//
private:
	int AddAgent(std::string const&  agentName, NavMeshAgent& agent);

	//give the agentName and the agentID in the navmeshagent return the corresponding DT crowdmap 
	int GetDTCrowdIndex(std::string const& agentName,int agentID );

	//Helper function to configure navMeshParams
	dtCrowdAgentParams ConfigureDTParams(NavMesh const& navMesh, NavMeshAgent const& navMeshAgent);
private:
	using AgentID = int;
	using dtCrowdID = int;
	using ArrayIndex = int;
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	
	std::unordered_map<std::string, TypedResourceID<NavMesh> > sceneNavMeshes;

	std::unordered_map<std::string, std::unique_ptr<dtNavMeshQuery, dtQueryDeleter> > queryManager;

	std::unordered_map<std::string, std::unique_ptr<dtCrowd,dtCrowdDeleter> > crowdManager;

	std::unordered_map<std::string, std::vector<AgentID>> agentList; //holds a the record of used and unused dtCrowd objects. LastIndex holds the value of last used element. 
																	//motivatation to optimise adding and removing agents from the dtcrowd

	std::unordered_map<std::string, int> lastIndex;

	std::unordered_map<std::string, std::unordered_map<AgentID, ArrayIndex>> agentToIndexMap; // takes a NavmeshAgent agentID and map it to a index in agentList. Age

	std::unordered_map<std::string, std::unordered_map<ArrayIndex, AgentID>> indexToAgentMap; // takes a holds the inverse copy of agentList index so that the last elements know which is tied to fro removale
	//std::unordered_map<std::string, std::unordered_map<ArrayIndex, AgentID>> indexToAgentMap; //man

	const int  agentLimit = 1000;

	int lastAvailable = 0;
	bool hasSystemInit = false; //check if system has init before adding any other agent

	ResourceID navMeshId;
};


struct navMeshPath
{
	std::vector<glm::vec3> hello;
};



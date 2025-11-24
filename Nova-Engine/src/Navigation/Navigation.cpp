#include "Navigation.h"
#include "Engine/engine.h"
#include "Logger.h"
#include "ResourceManager/resourceManager.h"
#include <numbers>
#include <algorithm>
#include <glm/gtx/fast_trigonometry.hpp>
#include "Profiling.h"

#undef min
#undef max

/*TO DO-----
  Handle entity on destroy
  handle runtime movement interruptions
  handle runtime data edition
  handle runtime registration
  allow force based movement?



*/

/* To Note: 
	Library recommends not to feed it any positional data after registration. 
	Either use their API to request movement or remove and add back agent



*/

NavigationSystem::NavigationSystem(Engine& engine) :
	engine			{ engine },
	resourceManager	{ engine.resourceManager },
	registry		{ engine.ecs.registry },
	navMeshId		{ INVALID_RESOURCE_ID }
{

	//registry.on_construct<NavMeshAgent>().connect<&NavigationSystem::AddAgentsToSystem>(*this);
	registry.on_destroy<NavMeshAgent>().connect<&NavigationSystem::RemoveAgentsFromSystem>(*this);


}

NavigationSystem::~NavigationSystem()
{
	sceneNavMeshes.clear();
	crowdManager.clear();
	queryManager.clear();
	
}

void NavigationSystem::update(float const& dt)
{
	ZoneScoped;

	dtCrowdAgentDebugInfo debugInfo;

	//allow dtPathfinding to update this frame
	for (auto it = crowdManager.begin(); it != crowdManager.end(); it++)
	{
		dtCrowd* crowdAgent = it->second.get();
		
		if (crowdAgent) {
			crowdAgent->update(dt, &debugInfo);
		}
	}

	//then get all new data an feedback into the transform
	for (auto&& [entity,entityData, transform, agent] : registry.view<EntityData, Transform, NavMeshAgent>().each())
	{
		if (!entityData.isActive || !engine.ecs.isComponentActive<NavMeshAgent>(entity))
			continue;

		auto iterator = crowdManager.find(agent.agentName);

		if (iterator == crowdManager.end()) {
			Logger::error("Name changed? Doesn't exist.");
			return;
		}

		//dtCrowdAgent* dtAgent = iterator->second->getEditableAgent(agent.agentIndex);

		dtCrowdAgent* dtAgent = iterator->second->getEditableAgent(GetDTCrowdIndex(agent.agentName,agent.agentIndex));

		if (dtAgent) {


			if (agent.updatePosition)
			{
				transform.position.x = dtAgent->npos[0];
				transform.position.y = dtAgent->npos[1];
				transform.position.z = dtAgent->npos[2];
			}

			if (transform.position.x != dtAgent->npos[0] || transform.position.y != dtAgent->npos[1] || transform.position.z != dtAgent->npos[2])
			{
				continue;
			}

			constexpr float EPS_SQ = 1e-6f;

			if ((dtAgent->vel[0] * dtAgent->vel[0] + dtAgent->vel[2] * dtAgent->vel[2]) < EPS_SQ)
			{
				continue;
			}


			//----Working code
			if (agent.updateRotation)
			{
				float desiredYaw = std::atan2(dtAgent->vel[0], dtAgent->vel[2]);

				glm::quat yawQuat = glm::angleAxis(desiredYaw, glm::vec3(0.0f, 1.0f, 0.0f));
				transform.rotation = yawQuat;
			}


			//----Testing Code
				// convert to radians/sec
			//const float angRad = glm::radians(agent.agentRotationSpeed) * dt;
			//if (!std::isfinite(angRad) || std::abs(angRad) < 1e-8f) return;

			//// create delta quaternion (rotate around world up)
			//glm::quat delta = glm::angleAxis(angRad, glm::vec3(0.0f, 1.0f, 0.0f));

			//// apply delta in world space (pre-multiply). Use post-multiply if you want local-space yaw.
			//glm::quat newRot = glm::normalize(delta * transform.rotation);

			//// sanity check
			//if (!(std::isfinite(newRot.x) && std::isfinite(newRot.y) &&
			//	std::isfinite(newRot.z) && std::isfinite(newRot.w)))
			//	return;

			//transform.rotation = newRot;


		}
	}
}

void NavigationSystem::setNewNavMesh(ResourceID p_navMeshId) {
	navMeshId = p_navMeshId;
}

ResourceID NavigationSystem::getNavMeshId() const {
	return navMeshId;
}

void NavigationSystem::NavigationDebug()
{
#if 0
	glm::vec3 targetPosition{};
	//float targetposition_arr[3];

	//Get transform info of target
	for (auto&& [entity, transform, navTest] : registry.view<Transform, NavigationTestTarget>().each())
	{
		/*targetposition[0] = transform.position[0];
		targetposition[1] = transform.position[1];
		targetposition[2] = transform.position[2];*/
		targetPosition = transform.position;

	}

	//get all agents to pathfind to object
	for (auto&& [entity, transform, agent] : registry.view<Transform, NavMeshAgent>().each())
	{
		
		//dtQueryFilter const* filter = crowdManager[agent.agentName]->getFilter(0); // configure include/exclude flags or costs if needed
		//float const* halfExtents = crowdManager[agent.agentName]->getQueryHalfExtents();
		//dtPolyRef nearestRef = 0;
		//float nearestPt[3];
		//if (dtStatusSucceed(queryManager[agent.agentName]->findNearestPoly(targetposition, halfExtents, filter, &nearestRef, nearestPt)) && nearestRef)
		//{
		//	float clamped[3];
		//	queryManager[agent.agentName]->closestPointOnPoly(nearestRef, targetposition, clamped, nullptr);
		//	// clamped[] is the navmesh-constrained closest point

		//	crowdManager[agent.agentName]->requestMoveTarget(agent.agentIndex, nearestRef, targetposition);

		//}


		setDestination(entity, targetPosition);
	}


#endif
}

ENGINE_DLL_API void NavigationSystem::AddAgentsToSystem(entt::registry&, entt::entity entityID)
{

	//if (hasSystemInit == false)
	//{
	//	return;
	//}

	////get component
	//auto&& [transform, navMeshAgent] = registry.try_get<Transform, NavMeshAgent>(entityID);

	//if (navMeshAgent == nullptr || transform == nullptr )
	//{
	//	Logger::error("Sum Ting Wong, Agent Attempted to be added to system without an Agent");
	//	return;
	//}

	//
	////check if a valid navmesh exist in this scene
	//if (sceneNavMeshes.find(navMeshAgent->agentName) == sceneNavMeshes.end())
	//{
	//	Logger::warn("Agent found in scene without a valid Navmesh");
	//	return;
	//}

	//auto&& [navMeshAsset, _] = resourceManager.getResource<NavMesh>(sceneNavMeshes[navMeshAgent->agentName]);

	//float pos[3] = { transform->position.x, transform->position.y, transform->position.z };

	//dtCrowdAgentParams params = ConfigureDTParams(*navMeshAsset, *navMeshAgent);

	////get unused index
	//navMeshAgent->agentIndex = AddAgent(navMeshAgent->agentName, *navMeshAgent);
	//int dtCrowdIndex = GetDTCrowdIndex(navMeshAgent->agentName, navMeshAgent->agentIndex);

	////Impt here, using index to access data
	//crowdManager[navMeshAgent->agentName].get()->addAgent(dtCrowdIndex, pos, &params);
	////agent.agentIndex = crowdManager[agent.agentName].get()->addAgent(pos, &params);
	//if (navMeshAgent->agentIndex < 0)
	//{
	//	Logger::warn("Failed to add NavMeshAgent to crowd");
	//}




}

ENGINE_DLL_API void NavigationSystem::RemoveAgentsFromSystem(entt::registry&, entt::entity entityID)
{
	if (hasSystemInit == false)
	{
		return;
	}


	//get component
	auto&& [transform, navMeshAgent] = registry.try_get<Transform, NavMeshAgent>(entityID);

	//set to remove agen from respective dtcrowd
	if (navMeshAgent == nullptr)
	{

		Logger::error("Sum Ting Wong, Agent deleted without navmash component!");
		return;
	}

	int lastElement  = lastIndex[navMeshAgent->agentName]-1;
	int mapperindex  = agentToIndexMap[navMeshAgent->agentName][navMeshAgent->agentIndex];
	int agentIDofLastElement = indexToAgentMap[navMeshAgent->agentName][lastElement];
	int dtCrowdIndex = agentList[navMeshAgent->agentName][mapperindex]; //GetDTCrowdIndex(navMeshAgent->agentName, navMeshAgent->agentIndex);

	//remove from dtcrowdsManager
	crowdManager[navMeshAgent->agentName]->removeAgent(dtCrowdIndex);

	//swap current and last
	std::swap(agentList[navMeshAgent->agentName][mapperindex], agentList[navMeshAgent->agentName][lastElement]);

	//update mapper position, last pos mapped index to new index, current mappedIndex now 
	agentToIndexMap[navMeshAgent->agentName][agentIDofLastElement] = mapperindex;
	indexToAgentMap[navMeshAgent->agentName][mapperindex] = agentIDofLastElement; //current occupied index is now held by agentID corresponding to last element

	//remove current mapper index, current now longer exist, and remove its corresponding map in indextoAgentMap
	agentToIndexMap[navMeshAgent->agentName].erase(navMeshAgent->agentIndex);
	indexToAgentMap[navMeshAgent->agentName].erase(lastElement); //remove id map to that last element cause position has switched corresponding to

	//reduce last index by one
	lastIndex[navMeshAgent->agentName]--;
}

ENGINE_DLL_API void NavigationSystem::InstantiateAgentsToSystem(entt::entity entityID, Transform const* const transform, NavMeshAgent* const navMeshAgent)
{
	if (hasSystemInit == false)
	{
		return;
	}

	//get component
	//auto&& [transform, navMeshAgent] = registry.try_get<Transform, NavMeshAgent>(entityID);
	//Assumed to be checked already
	if (navMeshAgent == nullptr || transform == nullptr)
	{
		Logger::error("Sum Ting Wong, Agent Attempted to be added to system without an Agent");
		return;
	}


	//check if a valid navmesh exist in this scene
	if (sceneNavMeshes.find(navMeshAgent->agentName) == sceneNavMeshes.end())
	{
		Logger::warn("Agent found in scene without a valid Navmesh");
		return;
	}

	auto&& [navMeshAsset, _] = resourceManager.getResource<NavMesh>(sceneNavMeshes[navMeshAgent->agentName]);

	float pos[3] = { transform->position.x, transform->position.y, transform->position.z };

	dtCrowdAgentParams params = ConfigureDTParams(*navMeshAsset, *navMeshAgent);

	//get unused index
	navMeshAgent->agentIndex = AddAgent(navMeshAgent->agentName, *navMeshAgent);
	int dtCrowdIndex = GetDTCrowdIndex(navMeshAgent->agentName, navMeshAgent->agentIndex);

	//Impt here, using index to access data
	crowdManager[navMeshAgent->agentName].get()->addAgent(dtCrowdIndex, pos, &params);
	//agent.agentIndex = crowdManager[agent.agentName].get()->addAgent(pos, &params);
	if (navMeshAgent->agentIndex < 0)
	{
		Logger::warn("Failed to add NavMeshAgent to crowd");
	}
}

ENGINE_DLL_API void NavigationSystem::SetAgentActive(entt::entity entityID)
{
	auto&& [transform, navMeshAgent] = engine.ecs.registry.try_get<Transform, NavMeshAgent>(entityID);

	if (navMeshAgent == nullptr || navMeshAgent->agentIndex < 0)
		return;
	int dtIndex = GetDTCrowdIndex(navMeshAgent->agentName, navMeshAgent->agentIndex);
    crowdManager[navMeshAgent->agentName]->getEditableAgent(dtIndex)->active = true;
	
	// Set Target Position to current transform so it wouldn't teleport back
	crowdManager[navMeshAgent->agentName]->setTargetPosition(navMeshAgent->agentIndex, transform->position.x, transform->position.y, transform->position.z);

}

ENGINE_DLL_API void NavigationSystem::SetAgentInactive(entt::entity entityID)
{
	auto&& navMeshAgent = registry.try_get<NavMeshAgent>(entityID);

	if (navMeshAgent == nullptr || navMeshAgent->agentIndex < 0)
		return;
	int dtIndex = GetDTCrowdIndex(navMeshAgent->agentName, navMeshAgent->agentIndex);
	crowdManager[navMeshAgent->agentName]->getEditableAgent(dtIndex)->active = false;
	crowdManager[navMeshAgent->agentName]->resetMoveTarget(dtIndex);
}

void NavigationSystem::initNavMeshSystems()
{
	
	//sceneNavMeshes.clear(); ----> Moved to Unload
	//crowdManager.clear();
	//queryManager.clear();

	unloadNavMeshSystems();

	//find all navmesh surfaces in this scene, set them to the system
	for (auto&& [entity, navMeshSurface] : registry.view<NavMeshSurface>().each())
	{
		if (!resourceManager.doesResourceExist(navMeshSurface.navMeshId))
		{
			Logger::error("Missing Navmesh file! Probably...");
			continue;
		}

		if (sceneNavMeshes.find(navMeshSurface.label) == sceneNavMeshes.end())
		{
			sceneNavMeshes.emplace(navMeshSurface.label ,  navMeshSurface.navMeshId);

			dtNavMeshQuery* dtQuery= dtAllocNavMeshQuery();
			queryManager.emplace(navMeshSurface.label, std::unique_ptr<dtNavMeshQuery,dtQueryDeleter>(dtQuery));


			dtCrowd* dtCrowdptr = dtAllocCrowd();
			crowdManager.emplace(navMeshSurface.label, std::unique_ptr<dtCrowd,dtCrowdDeleter>(dtCrowdptr));

			//Init crowd and query Manager
			auto&& [navMeshAsset, _] = resourceManager.getResource<NavMesh>(navMeshSurface.navMeshId);
			queryManager[navMeshSurface.label]->init(navMeshAsset->navMesh, 2048); 
			crowdManager[navMeshSurface.label]->init(agentLimit, navMeshAsset->buildRadius , navMeshAsset->navMesh);
			agentList[navMeshSurface.label].resize(agentLimit);

			for (int i = 0; i < agentList[navMeshSurface.label].size(); i++)
			{
				agentList[navMeshSurface.label][i] = i; //assign a dtCrowd agent ID to each slot at the start
			}

			lastIndex[navMeshSurface.label] = 0;
		}


	}

	//check system init to be true
	hasSystemInit = true;


	//for each active agent... register to its respective dt crowd
	for (auto&& [entity, transform ,agent] : registry.view<Transform,NavMeshAgent>().each())
	{

		//check if a valid navmesh exist in this scene
		if (sceneNavMeshes.find(agent.agentName) == sceneNavMeshes.end())
		{
			Logger::warn("Agent found in scene without a valid Navmesh");
			continue;
		}

		auto&& [navMeshAsset, _] = resourceManager.getResource<NavMesh>(sceneNavMeshes[agent.agentName]);

		float pos[3] = { transform.position.x, transform.position.y, transform.position.z };

		//dtCrowdAgentParams params{};
		//params.radius = navMeshAsset->buildRadius;
		//params.height = navMeshAsset->buildHeight;
		//params.maxSpeed = agent.agentMaxSpeed;
		//params.maxAcceleration = agent.agentMaxAcceleration;
		//params.collisionQueryRange = navMeshAsset->buildRadius * 12.f;
		//// pathOptimizationRange - choose a sensible default scaled from radius as in original code
		//params.pathOptimizationRange = navMeshAsset->buildRadius * 30.f;
		//params.obstacleAvoidanceType = 0; //default settings see dtCrowd init, its already set there currently we have on type only see how it goes first
		//params.updateFlags = UpdateFlags::DT_CROWD_ANTICIPATE_TURNS | UpdateFlags::DT_CROWD_SEPARATION | UpdateFlags::DT_CROWD_OPTIMIZE_VIS; //use for now
		//params.separationWeight = agent.separationWeight;


		dtCrowdAgentParams params =  ConfigureDTParams(*navMeshAsset, agent);

		////get unused index
		agent.agentIndex = AddAgent(agent.agentName, agent);
		int dtCrowdIndex = GetDTCrowdIndex(agent.agentName, agent.agentIndex);

		 //Impt here, using index to access data
		 crowdManager[agent.agentName].get()->addAgent(dtCrowdIndex,pos, &params);
		 //agent.agentIndex = crowdManager[agent.agentName].get()->addAgent(pos, &params);
		 if (agent.agentIndex < 0)
		 {
			 Logger::warn("Failed to add NavMeshAgent to crowd");
		 }

	}

}

ENGINE_DLL_API void NavigationSystem::unloadNavMeshSystems()
{
	sceneNavMeshes.clear();
	crowdManager.clear();
	queryManager.clear();
	agentToIndexMap.clear();
	agentList.clear();

	hasSystemInit = false;

}


bool NavigationSystem::setDestination(entt::entity entityID, glm::vec3 targetPosition)
{
	NavMeshAgent* agent = engine.ecs.registry.try_get<NavMeshAgent>(entityID);

	//--------------------Error Checking-----------------//
	if(!agent)
	{
		Logger::error("Entity {} does not have a NavMeshAgent", engine.ecs.registry.get<EntityData>(entityID).name );
		return false;
	}

	// Find crowd & query for this agent's navmesh label
	auto cIt = crowdManager.find(agent->agentName);
	auto qIt = queryManager.find(agent->agentName);
	if (cIt == crowdManager.end() || qIt == queryManager.end())
	{
		Logger::warn("NavMesh for {} not found. Did you forget to create one for the scene?", agent->agentName);
		return false;
	}

	float position[3] = { targetPosition.x,targetPosition.y,targetPosition.z };

//--------------------Path finding-----------------------------------------------------//

	//Finds the cloest point y from target position, but considers it too far if it strays from x and z position.
	dtQueryFilter const* filter = crowdManager[agent->agentName]->getFilter(0); // configure include/exclude flags or costs if needed
	float const* halfExtents = crowdManager[agent->agentName]->getQueryHalfExtents();
	dtPolyRef nearestRef = 0;
	float nearestPt[3];
	if (dtStatusSucceed(queryManager[agent->agentName]->findNearestPoly(position, halfExtents, filter, &nearestRef, nearestPt)) && nearestRef)
	{
		float clamped[3]; //true closest point
		queryManager[agent->agentName]->closestPointOnPoly(nearestRef, position, clamped, nullptr);

		//have some tolerance
		float horizontalTolerance = 0.01f; // fallback tolerance

		//distance check
		const float distanceX = clamped[0] - targetPosition.x;
		const float distanceZ = clamped[2] - targetPosition.z;

		//Too far in the X and Z direction!
		if ( (distanceX*distanceX + distanceZ * distanceZ )> (horizontalTolerance * horizontalTolerance))
		{
			return false;
		}

		int dtCrowdIndex = GetDTCrowdIndex(agent->agentName, agent->agentIndex);
		if (crowdManager[agent->agentName]->requestMoveTarget(dtCrowdIndex, nearestRef, clamped))
		{
			return true;
		}

		return true;
	}



	return false;
}

void NavigationSystem::stopAgent(entt::entity entityID)
{
	NavMeshAgent* agent = engine.ecs.registry.try_get<NavMeshAgent>(entityID);
	crowdManager[agent->agentName]->resetMoveTarget(agent->agentIndex);
}
int NavigationSystem::AddAgent(std::string const& agentName, NavMeshAgent& agent)
{
	//Failed insert
	if (lastIndex[agentName] + 1 > agentLimit)
	{
		return -1;
	}

	//check if an active index exist, return it instead
	if (agentToIndexMap[agentName].find(agent.agentIndex) != agentToIndexMap[agentName].end())
	{
		return agentToIndexMap[agentName][agent.agentIndex]; 
	}



	//access the agent list and find the last value. 
	int lastArrIndex = lastIndex[agentName];

	//agentList[agentName][lastArrIndex] = agent;
	//agentList[agentName][lastArrIndex]; //use the int stored in agentList as the dtCrowdIndex for this agent

	//recycle unused index, as id
	int unusedDTindex = agentList[agentName][lastArrIndex];

	agentToIndexMap[agentName].emplace(unusedDTindex, lastArrIndex); //assign this agent an mapped id pointing to this index in agentList as Id sport index can change but ID stay with obj
	indexToAgentMap[agentName].emplace(lastArrIndex, unusedDTindex); //assign the inverse so when removing last element we know what lastlement id is point too

	lastIndex[agentName]++; //increase list last elements
	return unusedDTindex;
}

int NavigationSystem::GetDTCrowdIndex(std::string const& agentName, int agentID)
{

	//get from mapper the mapped location
	int index = agentToIndexMap[agentName][agentID];
	return agentList[agentName][index];
}

dtCrowdAgentParams NavigationSystem::ConfigureDTParams(NavMesh const& navMesh, NavMeshAgent const& navMeshAgent)
{
	dtCrowdAgentParams params{};
	params.radius = navMesh.buildRadius;
	params.height = navMesh.buildHeight;
	params.maxSpeed = navMeshAgent.agentMaxSpeed;
	params.maxAcceleration = navMeshAgent.agentMaxAcceleration;
	params.collisionQueryRange = navMesh.buildRadius * 12.f;
	// pathOptimizationRange - choose a sensible default scaled from radius as in original code
	params.pathOptimizationRange = navMesh.buildRadius * 30.f;
	params.obstacleAvoidanceType = 0; //default settings see dtCrowd init, its already set there currently we have on type only see how it goes first
	params.updateFlags = UpdateFlags::DT_CROWD_ANTICIPATE_TURNS | UpdateFlags::DT_CROWD_SEPARATION | UpdateFlags::DT_CROWD_OPTIMIZE_VIS; //use for now
	params.separationWeight = navMeshAgent.separationWeight;

	return params;
}


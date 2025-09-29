#include "Navigation.h"
#include "Engine/engine.h"
#include "Logger.h"
#include "ResourceManager/resourceManager.h"
#include "component.h"

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
{}

NavigationSystem::~NavigationSystem()
{
	sceneNavMeshes.clear();
	crowdManager.clear();
	queryManager.clear();
	
}

void NavigationSystem::update(float const& dt)
{
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
	for (auto&& [entity, transform, agent] : registry.view<Transform, NavMeshAgent>().each())
	{
		dtCrowdAgent* dtAgent = crowdManager[agent.agentName].get()->getEditableAgent(agent.agentIndex);

		if (dtAgent) {
			transform.position.x = dtAgent->npos[0];
			transform.position.y = dtAgent->npos[1];
			transform.position.z = dtAgent->npos[2];
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

	glm::vec3 targetPosition;
	float targetposition_arr[3];

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


		setDestination(entity, targetPosition );

	}



}

ENGINE_DLL_API void NavigationSystem::initNavMeshSystems()
{
	sceneNavMeshes.clear();
	crowdManager.clear();
	queryManager.clear();

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
			crowdManager[navMeshSurface.label]->init(100 , navMeshAsset->buildRadius , navMeshAsset->navMesh);
		}


	}

	//for each active agent... register to its respective dt crowd, if automated is requested
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

		dtCrowdAgentParams params{};
		params.radius = navMeshAsset->buildRadius;
		params.height = navMeshAsset->buildHeight;
		params.maxAcceleration = agent.agentMaxAcceleration;
		params.maxSpeed = agent.agentMaxSpeed;
		params.collisionQueryRange = agent.collisionDetectionRange;
		// pathOptimizationRange - choose a sensible default scaled from radius as in original code
		params.pathOptimizationRange = navMeshAsset->buildRadius * 30.f;
		params.separationWeight = agent.separationWeight;


		//Impt here, using index to access data
		 agent.agentIndex = crowdManager[agent.agentName].get()->addAgent(pos, &params);
		 if (agent.agentIndex < 0)
		 {
			 Logger::warn("Failed to add NavMeshAgent to crowd");
		 }

	}

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


		if (crowdManager[agent->agentName]->requestMoveTarget(agent->agentIndex, nearestRef, clamped))
		{
			return false;
		}

		return true;
	}



	return false;
}


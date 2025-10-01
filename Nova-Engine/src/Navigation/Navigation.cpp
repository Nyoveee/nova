#include "Navigation.h"
#include "Engine/engine.h"
#include "Logger.h"
#include "ResourceManager/resourceManager.h"
#include "component.h"
#include <numbers>
#include <glm/gtx/fast_trigonometry.hpp>
#undef min;
#undef max;

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


			constexpr float EPS_SQ = 1e-6f;
			if ((dtAgent->vel[0] * dtAgent->vel[0] + dtAgent->vel[2] * dtAgent->vel[2]) < 1e-6f)
			{
				continue;
			}

			////get current facing direction of the agent
			//float currentYaw = transform.eulerAngles.angles.y;

			////glm::vec3 agentFacingDir = glm::normalize(glm::vec3(dtAgent->nvel[0], 0.0f, dtAgent->nvel[2]));
			//auto wrapPi = [](float a) {
			//	while (a > glm::pi<float>()) a -= 2.0f * glm::two_pi<float>();
			//	while (a <= -glm::pi<float>()) a += 2.0f * glm::two_pi<float>();
			//	return a;
			//};


			//float desiredYaw = std::atan2(dtAgent->vel[0], dtAgent->vel[2]);

			////glm::quat targetRotation = glm::angleAxis(desiredYaw, glm::vec3(0.f,1.f,0.f) );

			////shortest angular difference
			//float diff = wrapPi(desiredYaw - currentYaw);
			//
			//float frameRotation = glm::radians(agent.agentRotationSpeed) * dt; //get the rotation possible this frame


			////if the difference it too large this frame, use framerotation, else use difference to cover
			//if (std::fabs(diff) > frameRotation)
			//{
			//	diff = (diff > 0.0f) ? frameRotation : -frameRotation;
			//}

			//currentYaw += diff;

			//glm::quat yawQuat = glm::angleAxis(currentYaw, glm::vec3(0.0f, 1.0f, 0.0f));
			//transform.rotation = yawQuat;

			//glm::vec3 desiredFront{ dtAgent->vel[0],dtAgent->vel[1],dtAgent->vel[2] };
			//glm::vec3 currentFront{ transform.front };

			//glm::quat rotationQuat = glm::rotation(currentFront, desiredFront);

			//transform.rotation = transform.rotation * rotationQuat;


			//----Working code
			float desiredYaw = std::atan2(dtAgent->vel[0], dtAgent->vel[2]);

			glm::quat yawQuat = glm::angleAxis(desiredYaw, glm::vec3(0.0f, 1.0f, 0.0f));
			transform.rotation = yawQuat;


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

	glm::vec3 targetPosition{};
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

	//for each active agent... register to its respective dt crowd, if automated is requested, initialises agent parameters
	for (auto&& [entity, transform ,agent] : registry.view<Transform,NavMeshAgent>().each())
	{

		//check if a valid navmesh exist in this scene
		if (sceneNavMeshes.find(agent.agentName) == sceneNavMeshes.end())
		{
			Logger::warn("Agent found in scene without a valid Navmesh");
			continue;
		}
		agent.agentIndex = -1;

		auto&& [navMeshAsset, _] = resourceManager.getResource<NavMesh>(sceneNavMeshes[agent.agentName]);

		float pos[3] = { transform.position.x, transform.position.y, transform.position.z };

		dtCrowdAgentParams params{};
		params.radius = navMeshAsset->buildRadius;
		params.height = navMeshAsset->buildHeight;
		params.maxSpeed = agent.agentMaxSpeed;
		params.maxAcceleration = agent.agentMaxAcceleration;
		params.collisionQueryRange = navMeshAsset->buildRadius * 12.f;
		// pathOptimizationRange - choose a sensible default scaled from radius as in original code
		params.pathOptimizationRange = navMeshAsset->buildRadius * 30.f;
		params.obstacleAvoidanceType = 0; //default settings see dtCrowd init, its already set there currently we have on type only see how it goes first
		params.updateFlags = UpdateFlags::DT_CROWD_ANTICIPATE_TURNS | UpdateFlags::DT_CROWD_SEPARATION | UpdateFlags::DT_CROWD_OPTIMIZE_VIS; //use for now
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


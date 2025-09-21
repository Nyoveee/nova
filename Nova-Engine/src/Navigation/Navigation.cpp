#include "Navigation.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

NavigationSystem::NavigationSystem(Engine& engine) :
	engine			{ engine },
	resourceManager	{ engine.resourceManager },
	registry		{ engine.ecs.registry }
{
}

NavigationSystem::~NavigationSystem()
{
}

DLL_API dtNavMesh* NavigationSystem::RegisterNavigationMesh(std::string const& agentName, unsigned char* navData, int const& dataSize)
{

	//uniqueNavMeshPtr navMeshObj{dtAllocNavMesh()};
	//if (!navMeshObj)
	//{

	//	return nullptr;
	//}

	//navMeshObj->init(navData, dataSize, DT_TILE_FREE_DATA);

	//navMeshMap[agentName] = std::move(navMeshObj);

	//return ;
}

DLL_API void NavigationSystem::Terminate()
{
	return;
}


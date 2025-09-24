#include "Navigation.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

NavigationSystem::NavigationSystem(Engine& engine) :
	engine			{ engine },
	resourceManager	{ engine.resourceManager },
	registry		{ engine.ecs.registry },
	navMeshId		{ INVALID_RESOURCE_ID }
{}

NavigationSystem::~NavigationSystem()
{
}

void NavigationSystem::setNewNavMesh(ResourceID p_navMeshId) {
	navMeshId = p_navMeshId;
}

ResourceID NavigationSystem::getNavMeshId() const {
	return navMeshId;
}

void NavigationSystem::registerNavmeshData()
{



}


#include "Navigation.h"
#include "engine.h"
#include "assetManager.h"

NavigationSystem::NavigationSystem(Engine& engine) :
	engine{ engine },
	assetManager{ engine.assetManager },
	registry{ engine.ecs.registry },
	navData{ nullptr }
{
}

NavigationSystem::~NavigationSystem()
{
}

bool NavigationSystem::BuildNavMesh()
{



	return false;
}

#include "Navigation.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

NavigationSystem::NavigationSystem(Engine& engine) :
	engine			{ engine },
	resourceManager	{ engine.resourceManager },
	registry		{ engine.ecs.registry },
	navData			{ nullptr }
{
}

NavigationSystem::~NavigationSystem()
{
}


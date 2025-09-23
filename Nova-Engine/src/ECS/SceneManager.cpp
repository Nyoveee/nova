#include "SceneManager.h"
#include "Serialisation/serialisation.h"
#include "ECS.h"

#include "ResourceManager/resourceManager.h"

namespace {
	constexpr ResourceID NO_SCENE_LOADED = INVALID_RESOURCE_ID;
}

SceneManager::SceneManager(ECS& ecs, ResourceManager& resourceManager) :
	ecs				{ ecs },
	resourceManager	{ resourceManager },
	currentScene	{ NO_SCENE_LOADED }
{}

void SceneManager::loadScene(ResourceID id) {	
	if (currentScene == id) {
		return;
	}

	auto&& [scene, _] = resourceManager.getResource<Scene>(id);

	if (!scene) {
		Logger::error("Failed to load invalid scene w/ id {}", static_cast<std::size_t>(id));
		return;
	}

	ecs.registry.clear();
	currentScene = NO_SCENE_LOADED;
	
	Serialiser::deserialiseScene(ecs, scene->getFilePath().string.c_str());
	currentScene = scene->id();
}

ResourceID SceneManager::getCurrentScene() const {
	return currentScene;
}


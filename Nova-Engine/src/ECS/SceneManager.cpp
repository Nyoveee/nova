#include "SceneManager.h"
#include "Serialisation/serialisation.h"
#include "ECS.h"
#include "Engine/engine.h"

#include "ResourceManager/resourceManager.h"

SceneManager::SceneManager(ECS& ecs, Engine& engine, ResourceManager& resourceManager) :
	ecs				{ ecs },
	engine			{ engine },
	resourceManager	{ resourceManager },
	currentScene	{ NO_SCENE_LOADED }
{
	ecs.registry.on_construct<EntityData>().connect<&SceneManager::onEntityCreation>(*this);
	ecs.registry.on_destroy<EntityData>().connect<&SceneManager::onEntityDestruction>(*this);
}

void SceneManager::loadScene(ResourceID id) {	
	auto&& [scene, _] = resourceManager.getResource<Scene>(id);

	if (!scene) {
		Logger::error("Failed to load invalid scene w/ id {}", static_cast<std::size_t>(id));
		return;
	}

	ecs.registry.clear();
	currentScene = NO_SCENE_LOADED;
	
	Serialiser::deserialiseScene(ecs.registry, layers, scene->getFilePath().string.c_str());
	currentScene = scene->id();
	
#if 0
	// Once we have loaded the scene, we populate the layers according to it's layer id..
	for (entt::entity entity : layer.entities) {
		EntityData& entityData = ecs.registry.get<EntityData>(entity);

		if (entityData.layerId < 0 || entityData.layerId >= layers.size()) {
			Logger::warn("Entity {} had invalid layer. Resetting it..", entityData.name);
			entityData.layerId = 0;
		}

	}
#endif

	engine.SystemsOnLoad();
}

ResourceID SceneManager::getCurrentScene() const {
	return currentScene;
}

bool SceneManager::hasNoSceneSelected() const {
	return currentScene == NO_SCENE_LOADED;
}

void SceneManager::onEntityDestruction([[maybe_unused]] entt::registry& registry, [[maybe_unused]] entt::entity entityId) {
	EntityData& entityData = registry.get<EntityData>(entityId);

	layers[entityData.layerId].entities.erase(entityId);
}

void SceneManager::onEntityCreation(entt::registry& registry, entt::entity entityId) {
	EntityData& entityData = registry.get<EntityData>(entityId);

	if (entityData.layerId < 0 || entityData.layerId >= layers.size()) {
		Logger::warn("Entity {} had invalid layer. Resetting it..", entityData.name);
		entityData.layerId = 0;
	}

	layers[entityData.layerId].entities.insert(entityId);
}

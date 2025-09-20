#include "SceneManager.h"
#include "Serialisation/serialisation.h"
#include "ECS.h"

namespace {
	constexpr ResourceID NO_SCENE_LOADED = INVALID_RESOURCE_ID;
}

SceneManager::SceneManager(ECS& ecs) :
	ecs				{ ecs },
	currentScene	{ NO_SCENE_LOADED }
{}

void SceneManager::switchScene(Scene const& from, Scene const& to) {
	saveScene(from);
	loadScene(to);
}

void SceneManager::loadScene(Scene const& scene) {
	ecs.registry.clear();
	Serialiser::deserialiseScene(ecs, scene.getFilePath().string.c_str());
	currentScene = scene.id();
}

void SceneManager::saveScene(Scene const& scene) {
	Serialiser::serialiseScene(ecs, scene.getFilePath().string.c_str());
	currentScene = NO_SCENE_LOADED;
}


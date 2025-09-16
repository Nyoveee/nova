#include "SceneManager.h"
#include"Serialisation/serialisation.h"
#include <entt/entt.hpp>

namespace SceneManager {
	SceneManager::SceneManager(ECS& ecs, const char* fileName) {
		Serialiser::deserialiseScene(ecs, fileName);
	}

	void SceneManager::LoadScene(ECS& ecs, const char* inputFile, const char* outputFile)
	{
		Serialiser::serialiseScene(ecs, inputFile);
		
		entt::registry& registry = ecs.registry;
		registry.clear();

		Serialiser::deserialiseScene(ecs, outputFile);
	}
}



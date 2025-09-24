#pragma once

#include <fstream>

#include "scene.h"
#include "export.h"

class ECS;
class ResourceManager;
class Engine;

class SceneManager {
public:
	ENGINE_DLL_API SceneManager(ECS& ecs, Engine& engine ,ResourceManager& resourceManager);

public:

	ENGINE_DLL_API void loadScene(ResourceID id);

	ENGINE_DLL_API ResourceID getCurrentScene() const;

private:
	ECS& ecs;
	Engine& engine;
	ResourceManager& resourceManager;
	ResourceID currentScene;
};

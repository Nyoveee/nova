#pragma once

#include <fstream>

#include "scene.h"
#include "export.h"

class ECS;
class ResourceManager;
class Engine;

constexpr ResourceID NO_SCENE_LOADED = INVALID_RESOURCE_ID;

class SceneManager {
public:
	ENGINE_DLL_API SceneManager(ECS& ecs, Engine& engine ,ResourceManager& resourceManager);

public:

	ENGINE_DLL_API void loadScene(ResourceID id);

	ENGINE_DLL_API ResourceID getCurrentScene() const;
	ENGINE_DLL_API bool hasNoSceneSelected() const;

private:
	ECS& ecs;
	Engine& engine;
	ResourceManager& resourceManager;
	ResourceID currentScene;
};

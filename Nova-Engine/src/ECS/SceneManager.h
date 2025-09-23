#pragma once

#include <fstream>

#include "scene.h"
#include "export.h"

class ECS;

class SceneManager {
public:
	ENGINE_DLL_API SceneManager(ECS& ecs);
	//ENGINE_DLL_API ~SceneManager();

public:
	ENGINE_DLL_API void switchScene(Scene const& from, Scene const& to);

	ENGINE_DLL_API void loadScene(Scene const& scene);
	ENGINE_DLL_API void saveScene(Scene const& scene);
	ENGINE_DLL_API ResourceID getCurrentScene();

private:
	ECS& ecs;
	ResourceID currentScene;
};

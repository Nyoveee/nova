#pragma once

#include <fstream>

#include "scene.h"
#include "export.h"

class ECS;

class SceneManager {
public:
	FRAMEWORK_DLL_API SceneManager(ECS& ecs);
	
public:
	FRAMEWORK_DLL_API void switchScene(Scene const& from, Scene const& to);

	FRAMEWORK_DLL_API void loadScene(Scene const& scene);
	FRAMEWORK_DLL_API void saveScene(Scene const& scene);

private:
	ECS& ecs;
	ResourceID currentScene;
};

#pragma once

#include <fstream>

#include "scene.h"

class ECS;

class SceneManager {
public:
	SceneManager(ECS& ecs);
	
public:
	void switchScene(Scene const& from, Scene const& to);

	void loadScene(Scene const& scene);
	void saveScene(Scene const& scene);

private:
	ECS& ecs;
	ResourceID currentScene;
};

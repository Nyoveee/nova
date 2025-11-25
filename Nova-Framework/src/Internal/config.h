#pragma once

#include "reflection.h"
#include "type_alias.h"

struct GameConfig {
	std::string gameName		= "Ichorus";
	int			gameWidth		= 1920;
	int			gameHeight		= 1080;
	ResourceID	sceneStartUp	= INVALID_RESOURCE_ID;
	float		gravityStrength = 60.f;

	REFLECTABLE(
		gameName,
		gameWidth,
		gameHeight,
		sceneStartUp,
		gravityStrength
	)
};

struct EditorConfig {

};
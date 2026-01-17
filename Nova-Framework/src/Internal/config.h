#pragma once

#include "reflection.h"
#include "type_alias.h"

class CubeMap;

struct GameConfig {
	std::string					gameName				= "Ichorus";
	int							gameWidth				= 1920;
	int							gameHeight				= 1080;
	ResourceID					sceneStartUp			= INVALID_RESOURCE_ID;
	float						gravityStrength			= 60.f;
	
	TypedResourceID<CubeMap>	environmentDiffuseMap	= { INVALID_RESOURCE_ID };
	TypedResourceID<CubeMap>	environmentSpecularMap	= { INVALID_RESOURCE_ID };

	REFLECTABLE(
		gameName,
		gameWidth,
		gameHeight,
		sceneStartUp,
		gravityStrength,
		environmentDiffuseMap,
		environmentSpecularMap
	)
};

struct EditorConfig {

};
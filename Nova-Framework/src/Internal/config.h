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
	
	bool						fullScreen				= true;

	TypedResourceID<CubeMap>	environmentDiffuseMap	= { INVALID_RESOURCE_ID };
	TypedResourceID<CubeMap>	environmentSpecularMap	= { INVALID_RESOURCE_ID };

	REFLECTABLE(
		gameName,
		gameWidth,
		gameHeight,
		sceneStartUp,
		gravityStrength,
		fullScreen,
		environmentDiffuseMap,
		environmentSpecularMap
	)
};

enum class ToneMappingMethod {
	Exposure,
	Reinhard,
	ACES,
	None
};

struct RenderConfig {
	ToneMappingMethod	toneMappingMethod		= ToneMappingMethod::ACES;
	NormalizedFloat		iblDiffuseStrength		= 1.f;
	NormalizedFloat		iblSpecularStrength		= 1.f;
	bool				toEnableSSAO			= true;
	bool				toEnableFog				= true;
	bool				toEnableVsync			= true;
	bool				toEnableAntiAliasing	= true;
	bool				toEnableShadows			= true;
	bool				toEnableIBL				= true;

	REFLECTABLE(
		toneMappingMethod,
		iblDiffuseStrength,
		iblSpecularStrength,
		toEnableSSAO,
		toEnableFog,
		toEnableVsync,
		toEnableAntiAliasing,
		toEnableShadows,
		toEnableIBL
	)
};

struct EditorConfig {

};
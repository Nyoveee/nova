#pragma once

#include "reflection.h"
#include "type_alias.h"

class CubeMap;

enum class ToneMappingMethod {
	Exposure,
	Reinhard,
	ACES,
	None
};

struct GameConfig {
	std::string					gameName				= "Ichorus";
	int							gameWidth				= 1920;
	int							gameHeight				= 1080;
	ResourceID					sceneStartUp			= INVALID_RESOURCE_ID;
	float						gravityStrength			= 60.f;
	
	TypedResourceID<CubeMap>	environmentDiffuseMap	= { INVALID_RESOURCE_ID };
	TypedResourceID<CubeMap>	environmentSpecularMap	= { INVALID_RESOURCE_ID };

	ToneMappingMethod			toneMappingMethod		= ToneMappingMethod::ACES;
	NormalizedFloat				iblDiffuseStrength		= 1.f;
	NormalizedFloat				iblSpecularStrength		= 1.f;

	REFLECTABLE(
		gameName,
		gameWidth,
		gameHeight,
		sceneStartUp,
		gravityStrength,
		environmentDiffuseMap,
		environmentSpecularMap,
		toneMappingMethod,
		iblDiffuseStrength,
		iblSpecularStrength
	)
};

struct RenderConfig {
	bool				toEnableSSAO			= true;
	bool				toEnableFog				= false;
	bool				toEnableVsync			= true;
	bool				toEnableAntiAliasing	= true;
	bool				toEnableShadows			= true;
	bool				toEnableIBL				= true;
	bool				fullScreen				= true;

	REFLECTABLE(
		toEnableSSAO,
		toEnableFog,
		toEnableVsync,
		toEnableAntiAliasing,
		toEnableShadows,
		toEnableIBL,
		fullScreen
	)
};

struct EditorConfig {

};

struct AudioConfig {
	NormalizedFloat masterVolume = 1.f;
	NormalizedFloat bgmVolume = 1.f;
	NormalizedFloat sfxVolume = 1.f;

	REFLECTABLE(
		masterVolume,
		bgmVolume,
		sfxVolume
	)
};
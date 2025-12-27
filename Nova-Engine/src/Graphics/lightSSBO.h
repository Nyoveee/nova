#pragma once

#include "bufferObject.h"

// A light SSBO contains 3 SSBOs, for the respective point lights. 
// Game render and editor render will use a different LightSSBO, due to the difference in frustum culling and clustered rendering from different camera.
class LightSSBO {
public:
	LightSSBO(int maxNumOfLights);

	LightSSBO(LightSSBO  const& other)				= delete;
	LightSSBO(LightSSBO&& other)					= default;
	LightSSBO& operator=(LightSSBO  const& other)	= delete;
	LightSSBO& operator=(LightSSBO&& other)			= default;

public:
	BufferObject pointLight;
	BufferObject directionalLight;
	BufferObject spotLight;
};

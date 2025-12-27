#include "lightSSBO.h"
#include "vertex.h"

LightSSBO::LightSSBO(int maxAmountOfLights) :
	pointLight			{ static_cast<GLsizeiptr>(maxAmountOfLights * sizeof(PointLightData)		+ alignof(PointLightData)) },
	directionalLight	{ static_cast<GLsizeiptr>(maxAmountOfLights * sizeof(DirectionalLightData)	+ alignof(DirectionalLightData)) },
	spotLight			{ static_cast<GLsizeiptr>(maxAmountOfLights * sizeof(SpotLightData)			+ alignof(SpotLightData)) }
{}

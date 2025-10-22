#pragma once
#include <glm/glm.hpp>
#include "type_alias.h"
#include "export.h"
namespace Interpolation
{
	// Lerp function based on the formula x = y ^ degree
	// Setting the degree to one will just be another version of std::lerp
	// Range (0, infinite)
	FRAMEWORK_DLL_API float Interpolation(float start, float end, float t, float degree);
	FRAMEWORK_DLL_API glm::vec4 Interpolation(glm::vec4 start, glm::vec4 end, float t, float degree);
	FRAMEWORK_DLL_API glm::vec3 Interpolation(glm::vec3 start, glm::vec3 end, float t, float degree);
	FRAMEWORK_DLL_API glm::vec2 Interpolation(glm::vec2 start, glm::vec2 end, float t, float degree);
	FRAMEWORK_DLL_API ColorA Interpolation(ColorA start, ColorA end, float t, float degree);
	FRAMEWORK_DLL_API Color Interpolation(Color start, Color end, float t, float degree);
};


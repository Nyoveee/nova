#pragma once
#include <glm/glm.hpp>
#include "type_alias.h"
namespace Interpolation
{
	// Lerp function based on the formula x = y ^ degree
	// Setting the degree to one will just be another version of std::lerp
	float Interpolation(float start, float end, float t, float degree);
	glm::vec3 Interpolation(glm::vec3 start, glm::vec3 end, float t, float degree);
	glm::vec2 Interpolation(glm::vec2 start, glm::vec2 end, float t, float degree);
	ColorA Interpolation(ColorA start, ColorA end, float t, float degree);
	Color Interpolation(Color start, Color end, float t, float degree);
};


#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 textureUnit;
};
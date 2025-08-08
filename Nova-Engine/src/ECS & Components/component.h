#pragma once

#include <vector>
#include <glm/mat4x4.hpp>

#include "../Libraries/type_alias.h"
#include "../Graphics/vertex.h"

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4x4 modelMatrix;
};

struct Mesh {
	std::vector<Vertex> vertices;
	Color uniformColor;
};
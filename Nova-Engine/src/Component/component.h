#pragma once

#include <vector>
#include <glm/mat4x4.hpp>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;

	glm::mat4x4 modelMatrix;
};

struct ModelRenderer {
	AssetID modelId;
};
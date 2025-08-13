#pragma once

#include <vector>
#include <glm/mat4x4.hpp>
#include <unordered_map>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"

using MaterialName = std::string;

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;

	glm::mat4x4 modelMatrix;
};

struct MeshRenderer {
	struct Material {
		AssetID diffuseTextureId;
	};

	AssetID modelId;

	// maps a material name from the model to a specific material texture
	std::unordered_map<MaterialName, Material> materials;
};
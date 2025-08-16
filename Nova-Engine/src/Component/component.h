#pragma once

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <unordered_map>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"

using MaterialName = std::string;

struct EntityData {
	std::string name;
	entt::entity parent = entt::null;
	std::vector<entt::entity> children = {};
};

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;

	glm::mat4x4 modelMatrix;

	float test1;
	int test2;
};

struct MeshRenderer {
	struct Material {
		AssetID diffuseTextureId;
	};

	AssetID modelId;

	// maps a material name from the model to a specific material texture
	std::unordered_map<MaterialName, Material> materials;

	bool toRenderOutline = false;
};
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <unordered_map>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"

// Make sure your components are of aggregate type!!
// This means it extremely easy for systems to work with these components
// Components should only hold data! Let systems work on these components.

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

	glm::vec3 lastPosition;
	glm::vec3 lastScale;
	glm::vec3 lastRotation;

	glm::mat4x4 modelMatrix;			// model matrix represents the final matrix to change a object to world space.

	// ====== Hierarchy related data =======
	glm::mat4x4 localMatrix{};	// transformation matrix in respect to parent!

	glm::vec3 localPosition = {};
	glm::vec3 localScale	= { 1.f, 1.f, 1.f };
	glm::vec3 localRotation = {};

	// Dirty bit indicating whether we need to recalculate the model view matrix.
	// When first created set it to true.
	bool recentlyUpdated = true;
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
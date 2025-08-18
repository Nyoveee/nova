#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"
#include "Libraries/reflection.h"

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
	glm::vec3 scale			{ 1.0f, 1.0f, 1.0f };
	glm::quat rotation		{};

	glm::vec3 lastPosition	{ position };
	glm::vec3 lastScale		{ scale };
	glm::quat lastRotation	{ rotation };

	glm::mat4x4 modelMatrix;			// model matrix represents the final matrix to change a object to world space.

	// ====== Hierarchy related data =======
	glm::mat4x4 localMatrix		{};	// transformation matrix in respect to parent!

	glm::vec3 localPosition		{};
	glm::vec3 localScale		{ 1.f, 1.f, 1.f };
	glm::quat localRotation		{};

	glm::vec3 lastLocalPosition {};
	glm::vec3 lastLocalScale	{ 1.f, 1.f, 1.f };
	glm::quat lastLocalRotation	{};

	// Dirty bit indicating whether we need to recalculate the model view matrix.
	// When first created set it to true.
	bool worldHasChanged = true;
	bool needsRecalculating = false;
	
	// Reflect these data members for level editor to display
	REFLECTABLE(
		position,
		scale,
		rotation,
		localPosition,
		localScale,
		localRotation
	)
};

struct MeshRenderer {
	struct Material {
		AssetID diffuseTextureId;
	};

	AssetID modelId;

	// maps a material name from the model to a specific material texture
	std::unordered_map<MaterialName, Material> materials;

	bool toRenderOutline = false;

	REFLECTABLE(
		modelId,
		materials
	)
};
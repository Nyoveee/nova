#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <variant>
#include <optional>

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
	glm::mat3x3 normalMatrix;			// normal matrix is used to transform normals.

	EulerAngles eulerAngles		{ rotation };			// this will be derieved from quartenions
	EulerAngles lastEulerAngles	{ rotation };			

	// ====== Hierarchy related data =======
	glm::mat4x4 localMatrix		{};	// transformation matrix in respect to parent!

	glm::vec3 localPosition		{};
	glm::vec3 localScale		{ 1.f, 1.f, 1.f };
	glm::quat localRotation		{};

	glm::vec3 lastLocalPosition {};
	glm::vec3 lastLocalScale	{ 1.f, 1.f, 1.f };
	glm::quat lastLocalRotation	{};

	EulerAngles localEulerAngles { localRotation };	// this will be derieved from quartenions
	EulerAngles lastLocalEulerAngles{ localRotation };

	// Dirty bit indicating whether we need to recalculate the model view matrix.
	// When first created set it to true.
	bool worldHasChanged = true;
	bool needsRecalculating = false;
	
	// Reflect these data members for level editor to display
	REFLECTABLE(
		position,
		scale,
		rotation,
		eulerAngles,
		localPosition,
		localScale,
		localRotation,
		localEulerAngles
	)
};

struct Light {
	Color color;
	float intensity;

	REFLECTABLE(
		color,
		intensity
	)
};

struct Material {
	// either texture map or constant.
	std::variant<AssetID, Color>	albedo				= Color{ 0.1f, 0.1f, 0.1f };
	std::variant<AssetID, float>	roughness			= 0.5f;
	std::variant<AssetID, float>	metallic			= 0.f;
	std::optional<AssetID>			normalMap			= std::nullopt;
	std::optional<AssetID>			ambientOcculusion	= std::nullopt;

	float ambient = 0.2f;
};

struct MeshRenderer {
	AssetID modelId;

	// maps a material name from the model to a specific material texture
	std::unordered_map<MaterialName, Material> materials;

	bool toRenderOutline = false;

	REFLECTABLE(
		modelId,
		materials
	)
};
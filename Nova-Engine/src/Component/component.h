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

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "Libraries/type_alias.h"
#include "Graphics/vertex.h"
#include "Libraries/reflection.h"

// Forward declaring..
class Model;
class Texture;
class CubeMap;

// Make sure your components are of aggregate type!!
// This means it extremely easy for systems to work with these components
// Components should only hold data! Let systems work on these components.

// List all the component types. This is used as a variadic argument to certain functions.
#define ALL_COMPONENTS \
	EntityData, Transform, Light, MeshRenderer, Rigidbody, BoxCollider, SphereCollider, SkyBox, AudioComponent, Scripts

using MaterialName = std::string;
using ScriptName   = std::string;

struct EntityData {
	std::string name;
	entt::entity parent = entt::null;
	std::vector<entt::entity> children = {};
};

struct Transform {
	glm::vec3 position;
	glm::vec3 scale			{ 1.0f, 1.0f, 1.0f };
	glm::quat rotation		{ 1.0f, 0.f, 0.f, 0.f };

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
	glm::quat localRotation		{ 1.0f, 0.f, 0.f, 0.f };

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
	enum class Type : unsigned int {
		PointLight = 0,
		Directional = 1,
		Spotlight = 2
	};

	Color color = Color{ 1.f, 1.f, 1.f };
	float intensity = 1.f;
	Type type = Light::Type::PointLight;
	
	REFLECTABLE(
		type,
		color,
		intensity
	)
};

struct Material {
	enum class Pipeline {
		PBR,			// uses everything.
		BlinnPhong,		// use albedo and normal map.
		Color			// only uses albedo.
	};

	struct Config {
		float roughness;
		float metallic;
		float occulusion;
	};

	Pipeline renderingPipeline = Pipeline::BlinnPhong;

	// either texture map or constant.
	std::variant<AssetID, Color>	albedo		= Color{ 0.1f, 0.1f, 0.1f };
	std::variant<AssetID, Config>	config		= Config{ 0.5f, 0.f, 0.f };
	std::optional<AssetID>			normalMap	= std::nullopt;

	float ambient = 0.1f;
};

struct MeshRenderer {
	TypedAssetID<Model> modelId;

	// maps a material name from the model to a specific material texture
	std::unordered_map<MaterialName, Material> materials;

	bool toRenderOutline = false;

	REFLECTABLE(
		modelId,
		materials
	)
};

struct Rigidbody {
	JPH::EMotionType motionType;

	enum class Layer {
		NonMoving,
		Moving
	} layer;

	JPH::BodyID bodyId {}; // default constructed body ID is invalid.

	REFLECTABLE(
		motionType,
		layer
	)
};

struct BoxCollider {
	glm::vec3 scaleMultiplier { 1.f, 1.f, 1.f };
	bool scaleWithTransform = true;

	REFLECTABLE(
		scaleMultiplier,
		scaleWithTransform
	)
};

struct SphereCollider {
	float radius;

	REFLECTABLE(
		radius
	)
};

struct SkyBox {
	TypedAssetID<CubeMap> cubeMapId;
	
	REFLECTABLE(
		cubeMapId
	)
};
struct ScriptData
{
	std::string name;
	// Additional data includes those properties in the scripts
};
struct Scripts
{
	std::vector<ScriptData> scriptDatas;
	REFLECTABLE(
		scriptDatas
	)
};

struct AudioComponent {
	AssetID audio;

	REFLECTABLE(
		audio
	)
};
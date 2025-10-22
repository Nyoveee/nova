#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <array>

#include "type_alias.h"

#include "reflection.h"

#include "animation.h"
#include "skeleton.h"

#undef max

// We use index to represent vertices and bones..
using GlobalVertexIndex = unsigned int;		// global vertex index are like indices per mesh, but we offset by the size of the previous mesh.

using MaterialName		= std::string;

struct Vertex {
	glm::vec3 pos;
	glm::vec2 textureUnit;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	REFLECTABLE(
		pos,
		textureUnit,
		normal,
		tangent,
		bitangent
	)
};

struct SimpleVertex {
	glm::vec3 pos;
};
struct ParticleVertex {
	glm::vec3 localPos;
	glm::vec3 worldPos;
	glm::vec2 texCoord;
	glm::vec4 color;
	float rotation;
};

struct Mesh {
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::string materialName;

	int numOfTriangles;
	
	// contains bone information for skeletal animation..
	std::vector<VertexWeight> vertexWeights;

	REFLECTABLE(
		name,
		vertices,
		indices,
		materialName,
		numOfTriangles,
		vertexWeights
	)
};

// this is the model data that will be de/serialised.
struct ModelData {
	std::vector<Mesh> meshes;
	std::unordered_set<MaterialName> materialNames;

	// skeleton.
	std::optional<Skeleton> skeleton;

	// animations :)
	std::vector<Animation> animations;

	float maxDimension;

	REFLECTABLE(
		meshes,
		materialNames,
		skeleton,
		animations,
		maxDimension
	)
};

// this struct will be used to send to SSBO.
// our SSBO follows the std430 alignment rule,
// this caveat means that we have to be mightful of alignments of 
// our native types, especially for vec3s like color for an example.

#pragma warning( push )
#pragma warning(disable : 4324)			// disable warning about structure being padded, that's exactly what i wanted.

struct alignas(16) PointLightData {
	alignas(16) glm::vec3 lightPos;		// this will represent light position for point light
	alignas(16) glm::vec3 color;		// strength of the light, not limited to range of [0, 1]
	alignas(16) glm::vec3 attenuation;  // Constant, linear, quadratic values of attenuation
};

struct alignas(16) DirectionalLightData{
	alignas(16) glm::vec3 lightDir;		// this will represent light direction for directional light
	alignas(16) glm::vec3 color;		// strength of the light, not limited to range of [0, 1]
};

struct alignas(16) SpotLightData {
	alignas(16) glm::vec3 lightPos;		// this will represent light position for spot light
	alignas(16) glm::vec3 lightDir;		// this will represent light direction for spot light
	alignas(16) glm::vec3 color;		// strength of the light, not limited to range of [0, 1]
	alignas(16) glm::vec3 attenuation;  // Constant, linear, quadratic values of attenuation
	float cutOffAngle;					// Inner cutoff angle which shows full brightness
	float outerCutOffAngle;				// Outer cutoff angle where the light will dim from inner to outer range
};

#pragma warning( pop )

#if 0
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

	Pipeline renderingPipeline = Pipeline::PBR;

	// either texture map or constant.
	std::variant<ResourceID, Color>		albedo = Color{ 0.1f, 0.1f, 0.1f };
	std::variant<ResourceID, Config>	config = Config{ 0.5f, 0.f, 0.f };
	std::optional<ResourceID>			normalMap = std::nullopt;

	float ambient = 0.1f;
};
#endif
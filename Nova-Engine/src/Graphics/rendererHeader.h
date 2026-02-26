	#pragma once

#include "bufferObject.h"
#include "entt/entt.hpp"

struct Mesh;
class Material;
class CustomShader;
class Shader;

enum class MeshType {
	Normal,
	Skinned
};

enum class RenderQueueConfig {
	Normal,
	IgnoreTransparent	// used in shadow pass..
};

enum class DepthConfig {
	UseMaterial,
	NoWrite,
	Ignore
};

enum class BlendConfig {
	UseMaterial,
	Ignore
};

enum class RenderPass {
	DepthPrePass,
	ColorPass
};

// We group all buffer objects of a model to a struct, with each buffer object storing the vertex attribute.
struct MeshBOs {
	BufferObject positionsVBO			{ BufferObject{0} };	// VA 0
	BufferObject textureCoordinatesVBO	{ BufferObject{0} };	// VA 1
	BufferObject normalsVBO				{ BufferObject{0} };	// VA 2
	BufferObject tangentsVBO			{ BufferObject{0} };	// VA 3

	// Skeletal animation VBO..
	BufferObject skeletalVBO			{ BufferObject{0} };	// VA 4
	BufferObject EBO					{ BufferObject{0} };	// VA 5
};

// --------------------------------------------- 
// These are responsible for building a render queue..

// Stores entity specific information..
struct EntityBatch {
	entt::entity entity;
	std::reference_wrapper<const Model> model;
	MeshType meshType;
	std::vector<std::reference_wrapper<const Mesh>> meshes;
};

// Stores material specific information..
struct MaterialBatch {
	std::reference_wrapper<const Material> material;
	std::reference_wrapper<const CustomShader> customShader;
	std::reference_wrapper<const Shader> shader;

	std::vector<EntityBatch> entities;
	int layerIndex;		// we want to differentiate render passes of different layer, even if material is the same.
};

// We can't batch transparent materials together, because we need to sort it..
// Therefore, we batch models first, then materials.. (opposite of opaque queue..)
struct TransparentMaterial {
	std::reference_wrapper<const Material> material;
	std::reference_wrapper<const CustomShader> customShader;
	std::reference_wrapper<const Shader> shader;

	std::vector<std::reference_wrapper<const Mesh>> meshes;
};

struct TransparentEntry {
	entt::entity entity;
	MeshType meshType;
	std::reference_wrapper<const Model> model;

	// Material information..
	std::vector<TransparentMaterial> materials;

	// our sorting requirement..
	float distanceToCamera;
};

struct RenderQueue {
	std::vector<MaterialBatch>	  opaqueMaterials;
	std::vector<TransparentEntry> transparentMaterials;

	// we map material id to index of the respective vectors.. (layer)..
	std::vector<std::unordered_map<ResourceID, int>> materialResourceIdToOpaqueIndex;
};

// --------------------------------------------- 

struct ShadowModelBatch {
	entt::entity entity;
	MeshType meshType;
	float modelScale;
	std::vector<std::reference_wrapper<const Mesh>> meshes;
};

struct ShadowRenderQueue {
	std::vector<ShadowModelBatch> modelBatches;
};

// --------------------------------------------- 

enum class RenderMode {
	Editor,
	Game
};
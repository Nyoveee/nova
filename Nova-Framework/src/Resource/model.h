#pragma once

#include <vector>
#include <unordered_set>
#include "vertex.h"
#include "resource.h"
#include "animation.h"

// If data members do not require explicit memory management, move semantics can be defaulted.
// ModelAsset is an asset specific to the asset manager wrapping a model and indicates whether it is loaded or not.
// Model is the a struct containing actual data for rendering.

class Model : public Resource {
public:
	FRAMEWORK_DLL_API Model(ResourceID id, ResourceFilePath resourceFilePath, ModelData modelData);
	
	FRAMEWORK_DLL_API ~Model() = default;
	FRAMEWORK_DLL_API Model(Model const& other) = delete;
	FRAMEWORK_DLL_API Model(Model&& other) = default;
	FRAMEWORK_DLL_API Model& operator=(Model const& other) = delete;
	FRAMEWORK_DLL_API Model& operator=(Model&& other) = default;

public:
	// Model data.
	std::vector<Mesh> meshes;
	std::vector<MaterialName> materialNames;

	// Skeleton.
	std::optional<Skeleton> skeleton;

	// Animation.
	std::vector<Animation> animations;

	float maxDimension;
	float scale;
};

template <>
struct AssetInfo<Model> : public BasicAssetInfo {
	float scale = 1.f;
};
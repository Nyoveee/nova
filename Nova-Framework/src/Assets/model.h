#pragma once

#include <vector>
#include <unordered_set>
#include "vertex.h"
#include "resource.h"

// If data members do not require explicit memory management, move semantics can be defaulted.
// ModelAsset is an asset specific to the asset manager wrapping a model and indicates whether it is loaded or not.
// Model is the a struct containing actual data for rendering.

using MaterialName = std::string;

class Model : public Resource {
public:
	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string materialName;
		int numOfTriangles;
	};

public:
	FRAMEWORK_DLL_API Model(ResourceID id, std::vector<Mesh> meshes, std::unordered_set<MaterialName> materialNames);
	
	FRAMEWORK_DLL_API ~Model() = default;
	FRAMEWORK_DLL_API Model(Model const& other) = delete;
	FRAMEWORK_DLL_API Model(Model&& other) = default;
	FRAMEWORK_DLL_API Model& operator=(Model const& other) = delete;
	FRAMEWORK_DLL_API Model& operator=(Model&& other) = default;

public:
	std::vector<Mesh> meshes;
	std::unordered_set<MaterialName> materialNames;

	float maxDimension;
};
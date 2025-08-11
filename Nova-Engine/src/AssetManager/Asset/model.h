#pragma once

#include <vector>
#include <unordered_set>
#include "Graphics/vertex.h"
#include "asset.h"
#include "Component/component.h"

// If data members do not require explicit memory management, move semantics can be defaulted.
// ModelAsset is an asset specific to the asset manager wrapping a model and indicates whether it is loaded or not.
// Model is the a struct containing actual data for rendering.

using MaterialName = std::string;

class Model : public Asset {
public:
	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string materialName;
		int numOfTriangles;
	};

public:
	Model(std::string filepath);
	
	~Model() = default;
	Model(Model const& other) = delete;
	Model(Model&& other) = default;
	Model& operator=(Model const& other) = delete;
	Model& operator=(Model&& other) = default;

public:
	void load() final;
	void unload() final;

public:
	std::vector<Mesh> meshes;
	std::unordered_set<MaterialName> materialNames;

	float maxDimension;
};
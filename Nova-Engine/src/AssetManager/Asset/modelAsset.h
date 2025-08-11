#pragma once

#include <vector>
#include "Graphics/vertex.h"
#include "asset.h"
#include "Component/component.h"

// If data members do not require explicit memory management, move semantics can be defaulted.
// ModelAsset is an asset specific to the asset manager wrapping a model and indicates whether it is loaded or not.
// Model is the a struct containing actual data for rendering.

class ModelAsset : public Asset {
public:
	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		int numOfTriangles;
	};

	struct Model {
		std::vector<Mesh> meshes;
		float maxDimension;

		void clear() {
			meshes.clear();
		}
	};

public:
	ModelAsset(std::string filepath);
	
	~ModelAsset() = default;
	ModelAsset(ModelAsset const& other) = delete;
	ModelAsset(ModelAsset&& other) = default;
	ModelAsset& operator=(ModelAsset const& other) = delete;
	ModelAsset& operator=(ModelAsset&& other) = default;

public:
	void load() final;
	void unload() final;

public:
	Model model;
};
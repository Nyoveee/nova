#pragma once
#include <string>
#include <optional>
#include <assimp/material.h>

#include "model.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;

class ModelLoader {
public:
	struct ModelData {
		std::vector<Model::Mesh> meshes;
		std::unordered_set<MaterialName> materialNames;

		float maxDimension;
	};

	static std::optional<ModelData> loadModel(std::string const& filepath);

private:
	static Model::Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension);
};
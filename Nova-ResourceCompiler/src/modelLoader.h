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
	// kinda the same definition as the one defined in model.h (minus the asset inheritance)
	// but we need a separate class because we cannot modify the model instance directly
	// as we are loading this in another thread.
	struct ModelData {
		std::vector<Model::Mesh> meshes;
		std::unordered_set<MaterialName> materialNames;

		float maxDimension;
	};

	static std::optional<ModelData> loadModel(std::string const& filepath);

private:
	static Model::Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension);
};
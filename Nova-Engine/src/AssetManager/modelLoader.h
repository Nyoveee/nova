#pragma once
#include <string>
#include <optional>
#include <assimp/material.h>

#include "Asset/modelAsset.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;

class ModelLoader {
public:
	ModelLoader();

	std::optional<ModelAsset::Model> loadModel(std::string const& filepath) const;

private:
	ModelAsset::Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension) const;
	
	// get textures of a specific type from a given material
	void getTextures(aiMaterial* material, aiTextureType type) const;
};
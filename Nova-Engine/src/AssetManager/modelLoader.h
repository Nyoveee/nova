#pragma once
#include <string>
#include <optional>
#include <assimp/material.h>

#include "Asset/model.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;

class ModelLoader {
public:
	ModelLoader();

	bool loadModel(Model& model) const;

private:
	Model::Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension) const;
	
	// get textures of a specific type from a given material
	//void getTextures(aiMaterial* material, aiTextureType type) const;
};
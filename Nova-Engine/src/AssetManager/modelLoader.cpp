#include <Assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <algorithm>
#include <iostream>
#include "Graphics/vertex.h"

#include "modelLoader.h"
#include "Component/component.h"

namespace {
	glm::vec3 toGlmVec3(aiVector3D vec3) {
		return { vec3.x, vec3.y, vec3.z };
	}

	glm::vec2 toGlmVec2(aiVector3D vec3) {
		return { vec3.x, vec3.y };
	}

	glm::vec2 toGlmVec2(aiVector2D vec2) {
		return { vec2.x, vec2.y };
	}
}

ModelLoader::ModelLoader() {}

constexpr unsigned int PostProcessingFlags { 
	aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs 
};

bool ModelLoader::loadModel(Model& model) const {
	Assimp::Importer importer;

	aiScene const* scene = importer.ReadFile(model.getFilePath(), PostProcessingFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "Error when importing model: " << importer.GetErrorString() << '\n';
		return false;
	}

	std::vector<Model::Mesh> meshes;
	meshes.reserve(scene->mNumMeshes);

	// This records the max width (or height or length) of a model in an attempts to normalize it.
	float maxDimension = 0;

	// Iterate through all the meshes in a scene..
	for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
		meshes.push_back(processMesh(scene->mMeshes[i], scene, maxDimension));
		model.materialNames.insert(meshes[i].materialName);

		std::cout << meshes[i].materialName << "\n";
	}

	model.meshes = std::move(meshes);
	model.maxDimension = maxDimension;

	return true;
}

Model::Mesh ModelLoader::processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension) const {
	// Get all the vertices data in this mesh.
	std::vector<Vertex> vertices;
	vertices.reserve(mesh->mNumVertices);

	// Getting vertex attributes..
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		// 1. Position
		glm::vec3 position = toGlmVec3(mesh->mVertices[i]);

		if (position.x > maxDimension || position.y > maxDimension || position.z > maxDimension) {
			maxDimension = std::max(std::max(position.x, position.y), position.z);
		}

		// 2. Texture Coordinates
		glm::vec2 textureCoords;
		if (mesh->mTextureCoords[0]) {
			textureCoords = toGlmVec2(mesh->mTextureCoords[0][i]);
		}
		else {
			textureCoords = glm::vec2(0.0f, 0.0f);
		}
		
		vertices.push_back({ position, textureCoords });
	}

	// Getting indices..
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
	
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	std::string materialName;

	// Getting material info..
	if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		materialName = material->GetName().C_Str();
	}

	return { std::move(vertices), std::move(indices), std::move(materialName), static_cast<int>(mesh->mNumFaces)};
}

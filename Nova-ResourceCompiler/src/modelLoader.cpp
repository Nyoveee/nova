#include <Assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <algorithm>
#include <iostream>
#include "vertex.h"

#include "modelLoader.h"
#include "Logger.h"

#undef max

namespace {
	// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/assimp_glm_helpers.h
	glm::mat4 toGlmMat4(const aiMatrix4x4& from) {
		glm::mat4 to;

		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		
		return to;
	}

	glm::quat toGlmQuat(const aiQuaternion& pOrientation) {
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}

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

std::optional<ModelData> ModelLoader::loadModel(std::string const& filepath) {
	constexpr unsigned int PostProcessingFlags {
		aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	};

	Assimp::Importer importer;

	aiScene const* scene = importer.ReadFile(filepath, PostProcessingFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		Logger::error("Error when importing model with filepath {} : {}", filepath, importer.GetErrorString());
		return std::nullopt;
	}

	bones.clear();
	boneNameToIndex.clear();

	std::vector<Mesh> meshes;
	std::unordered_set<MaterialName> materialNames;

	meshes.reserve(scene->mNumMeshes);

	// This records the max width (or height or length) of a model in an attempts to normalize it.
	float maxDimension = 0;

	// Iterate through all the meshes in a scene..
	unsigned int vertexOffset = 0;

	for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
		meshes.push_back(processMesh(scene->mMeshes[i], scene, maxDimension, vertexOffset));
		vertexOffset += static_cast<unsigned int>(meshes[i].vertices.size());

		materialNames.insert(meshes[i].materialName);
	}

	// Process node hirerchy..
	rootBone = NO_BONE;
	processNodeHierarchy(scene->mRootNode);

#if 0
	for (auto& mesh : meshes) {
		for (auto& bone : mesh.bones) {
			Logger::info("Bone: {}", bone.name);

			for (auto& vertexWeight : bone.vertexWeights) {
				auto&& [index, weight] = vertexWeight;

				Logger::info("    vertex: {}, weight: {}", index, weight);
			}

			Logger::info("");
		}
	}
#endif

#if 1
	printBone(rootBone, 0);
#endif

	return {{ std::move(meshes), std::move(materialNames), std::move(bones), rootBone, maxDimension }};
}

Mesh ModelLoader::processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension, [[maybe_unused]] unsigned int vertexOffset) {
	// ==================================== 
	// Getting vertex attributes..
	// 1. position
	// 2. texture coordinates
	// 3. normal
	// 4. tangent
	// 5. bitangent
	// ==================================== 
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

		// 3. Normal
		glm::vec3 normal;
		if (mesh->mNormals) {
			normal = toGlmVec3(mesh->mNormals[i]);
		}
		else {
			normal = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		// 4. Tangents
		glm::vec3 tangent;
		if (mesh->mTangents) {
			tangent = toGlmVec3(mesh->mTangents[i]);
		}
		else {
			tangent = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		// 5. Bitangents
		glm::vec3 bitangent;
		if (mesh->mBitangents) {
			bitangent = toGlmVec3(mesh->mBitangents[i]);
		}
		else {
			bitangent = glm::vec3{ 0.0f, 0.0f, 0.f };
		}
		
		vertices.push_back({ position, textureCoords, normal, tangent, bitangent });
	}

	// ==================================== 
	// Getting indices for IBO (glDrawElement). 
	// Vertices were loaded in order.
	// ====================================
	
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
	
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	std::string materialName;

	// ==================================== 
	// Getting material information..
	// ==================================== 

	if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		materialName = material->GetName().C_Str();
	}

	// ==================================== 
	// For every bone, it contains most importantly..
	// - name
	// - what verticies it affects (vertex weight)
	// - offset matrix!
	// 
	// We need to map vertex indices back to the bone index too..
	// 
	// Bones are not unique to each mesh. We need to find the corresponding bone index, or generate a new one if it doesn't exist.
	// ==================================== 
	
	std::vector<VertexWeight> vertexWeights;

	// it is a skinned mesh.
	if (mesh->mNumBones) {
		bones.reserve(bones.size() + mesh->mNumBones);
		vertexWeights.resize(mesh->mNumVertices);

		BoneIndex boneIndex;

		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			aiBone const* bone = mesh->mBones[i];

			std::string boneName = bone->mName.C_Str();
			glm::mat4x4 offsetMatrix = toGlmMat4(bone->mOffsetMatrix);

			// We first see if this bone exist..
			if (auto iterator = boneNameToIndex.find(boneName); iterator != boneNameToIndex.end()) {
				boneIndex = iterator->second;
			}
			// bone doesnt exist, we use a new index.
			else {
				boneIndex = static_cast<BoneIndex>(bones.size());
				bones.push_back({ boneName, std::move(offsetMatrix) });
				boneNameToIndex.insert({ boneName, boneIndex });
			}

			// we retrieve vertex weight..
			for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
				auto aiVertexWeight = bone->mWeights[j];
				
				if (!vertexWeights[aiVertexWeight.mVertexId].addBone(boneIndex, aiVertexWeight.mWeight)) {
					Logger::warn("Failed to add bone, too many bones.");
				}
			}
		}
	}

	return { 
		mesh->mName.C_Str(),
		std::move(vertices), 
		std::move(indices), 
		std::move(materialName), 
		static_cast<int>(mesh->mNumFaces), 
		std::move(vertexWeights),
	};
}

void ModelLoader::processNodeHierarchy(aiNode const* node) {
	// process bone hierarchy...
	auto boneIterator = boneNameToIndex.find(node->mName.C_Str());

	if (boneIterator != boneNameToIndex.end()) {
		auto&& [_, boneIndex] = *boneIterator;
		BoneIndex parentBoneIndex = findParentBone(node->mParent);
		 
		if (parentBoneIndex != NO_BONE && boneIndex != rootBone) {
			bones[boneIndex].parentBone = parentBoneIndex;
			bones[parentBoneIndex].boneChildrens.push_back(boneIndex);
		}
		else {
			// we found our root bone.
			bones[boneIndex].parentBone = NO_BONE;
			rootBone = boneIndex;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNodeHierarchy(node->mChildren[i]);
	}
}

void ModelLoader::printBone(BoneIndex boneIndex, unsigned int padding) {
	if (boneIndex == NO_BONE) {
		return;
	}

	std::cout << '|';

	for (unsigned int i = 0; i < padding; ++i) {
		std::cout << ' ';
	}

	std::cout << "Bones: " << bones[boneIndex].name << '\n';

	++padding;

	for (auto& boneChildrenIndex : bones[boneIndex].boneChildrens) {
		printBone(boneChildrenIndex, padding);
	}
}

BoneIndex ModelLoader::findParentBone(aiNode const* parentNode) {
	if (parentNode) {
		auto parentBoneIterator = boneNameToIndex.find(parentNode->mName.C_Str());

		if (parentBoneIterator != boneNameToIndex.end()) {
			// we traversed to the root bone.
			//if (parentBoneIterator->second == rootBone) {
			//	return NO_BONE;
			//}

			return parentBoneIterator->second;
		}
		else {
			return findParentBone(parentNode->mParent);
		}
	}

	return NO_BONE;
}

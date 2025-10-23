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

	if (!scene || !scene->mRootNode) {
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
	for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
		meshes.push_back(processMesh(scene->mMeshes[i], scene, maxDimension));
		materialNames.insert(meshes[i].materialName);
	}

	// Process animation data..
	std::vector<Animation> animations;

	for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
		animations.push_back(processAnimation(scene->mAnimations[i]));
	}

	std::optional<Skeleton> skeletonOpt;

	// Process node hirerchy.. (if this model has bones..)
	if (bones.size()) {
		Skeleton skeleton {};
		skeleton.bones = std::move(bones);

		// start recursingly processing..
		processNodeHierarchy(skeleton, scene->mRootNode, NO_NODE);
		skeletonOpt = std::move(skeleton);
	}

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

#if 0
	printBone(rootBone, 0);
#endif

	return {{ std::move(meshes), std::move(materialNames), std::move(skeletonOpt), std::move(animations), maxDimension }};
}

Mesh ModelLoader::processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension) {
	// ==================================== 
	// Getting vertex attributes..
	// 1. position
	// 2. texture coordinates
	// 3. normal
	// 4. tangent
	// ==================================== 
	
	std::vector<glm::vec3> positions;
	positions.reserve(mesh->mNumVertices);
	
	std::vector<glm::vec2> textureCoordinates;
	textureCoordinates.reserve(mesh->mNumVertices);

	std::vector<glm::vec3> normals;
	normals.reserve(mesh->mNumVertices);

	std::vector<glm::vec3> tangents;
	tangents.reserve(mesh->mNumVertices);

	// Getting vertex attributes..
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		// 1. Position
		glm::vec3 position = toGlmVec3(mesh->mVertices[i]);

		if (position.x > maxDimension || position.y > maxDimension || position.z > maxDimension) {
			maxDimension = std::max(std::max(position.x, position.y), position.z);
		}

		positions.push_back(position);

		// 2. Texture Coordinates
		glm::vec2 textureCoords;

		if (mesh->mTextureCoords[0]) {
			textureCoords = toGlmVec2(mesh->mTextureCoords[0][i]);
		}
		else {
			textureCoords = glm::vec2(0.0f, 0.0f);
		}

		textureCoordinates.push_back(textureCoords);

		// 3. Normal
		glm::vec3 normal;

		if (mesh->mNormals) {
			normal = toGlmVec3(mesh->mNormals[i]);
		}
		else {
			normal = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		normals.push_back(normal);

		// 4. Tangents
		glm::vec3 tangent;

		if (mesh->mTangents) {
			tangent = toGlmVec3(mesh->mTangents[i]);
		}
		else {
			tangent = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		tangents.push_back(tangent);
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
		std::move(positions),
		std::move(textureCoordinates),
		std::move(normals),
		std::move(tangents),
		std::move(indices), 
		std::move(materialName), 
		static_cast<int>(mesh->mNumFaces), 
		std::move(vertexWeights),
	};
}

void ModelLoader::processNodeHierarchy(Skeleton& skeleton, aiNode const* node, ModelNodeIndex parentNodeIndex) {
	// process node hierarchy..

	// we verify if the current node is a bone...
	auto boneIterator = boneNameToIndex.find(node->mName.C_Str());

	bool isBone = false;
	BoneIndex nodeBoneIndex = NO_BONE;

	// we perform bone specific data handling..
	if (boneIterator != boneNameToIndex.end()) {
		auto&& [_, boneIndex] = *boneIterator;
		isBone = true;
		nodeBoneIndex = boneIndex;

		if (skeleton.rootBone == NO_BONE) {
			// We found our root bone.
			skeleton.bones[boneIndex].parentBone = NO_BONE;
			skeleton.rootBone = boneIndex;
		}
		else {
			// store bone hirerachy information..
			BoneIndex parentBoneIndex = findParentBone(node->mParent);

			if (parentBoneIndex != NO_BONE && boneIndex != skeleton.rootBone) {
				skeleton.bones[boneIndex].parentBone = parentBoneIndex;
				skeleton.bones[parentBoneIndex].boneChildrens.push_back(boneIndex);
			}
		}
	}
	
	// we handle node hierarchy here..
	ModelNodeIndex modelNodeIndex = static_cast<ModelNodeIndex>(skeleton.nodes.size());	// get an appropriate index for node.
	
	ModelNode modelNode {
		node->mName.C_Str(),
		toGlmMat4(node->mTransformation),
		isBone,
		nodeBoneIndex
	};

	// no root node yet..
	[[unlikely]] if (skeleton.rootNode == NO_NODE) {
		skeleton.rootNode = modelNodeIndex;
	}

	skeleton.nodes.push_back(std::move(modelNode));
	
	// set up relationship..
	skeleton.nodes[modelNodeIndex].parentNode = parentNodeIndex;

	[[likely]] if (parentNodeIndex != NO_NODE) {
		skeleton.nodes[parentNodeIndex].nodeChildrens.push_back(modelNodeIndex);
	}
	
	// recurse downwards.
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		aiNode const* childNode = node->mChildren[i];
		processNodeHierarchy(skeleton, childNode, modelNodeIndex);
	}
}

void ModelLoader::printBone([[maybe_unused]] BoneIndex boneIndex, [[maybe_unused]] unsigned int padding) {
#if 0
	auto printPadding = [](unsigned int padding) {
		for (unsigned int i = 0; i < padding; ++i) {
			std::cout << ' ';
		}
	};

	auto printMatrix = [&](glm::mat4x4 const& matrix) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				printPadding(padding);
				std::cout << std::left << std::setw(6) << std::fixed << std::setprecision(2) << matrix[j][i] << " "; // Column-major access
			}
			printPadding(padding);
			std::cout << std::endl;
		}
	};

	if (boneIndex == NO_BONE) {
		return;
	}

	std::cout << '|';

	printPadding(padding);
	std::cout << "Bones: " << bones[boneIndex].name << "\n\n";

	printMatrix(bones[boneIndex].offsetMatrix);
	std::cout << std::endl;
	printMatrix(skeleton.bones[boneIndex].transformationMatrix);

	padding += 4;

	for (auto& boneChildrenIndex : bones[boneIndex].boneChildrens) {
		printBone(boneChildrenIndex, padding);
	}
#endif
}

Animation ModelLoader::processAnimation(aiAnimation const* animation) {
	// an animation channel correspond to a bone name.
	std::unordered_map<std::string, AnimationChannel> animationChannel;

	// let's iterate through all the channels..
	for (unsigned int i = 0; i < animation->mNumChannels; ++i) {
		aiNodeAnim const* aiChannel = animation->mChannels[i];

		// ---- extract the 3 respective property.. ---- 
		// 1. position
		std::vector<VectorKey> positionKeys;
		positionKeys.reserve(aiChannel->mNumPositionKeys);

		for (unsigned j = 0; j < aiChannel->mNumPositionKeys; j++) {
			aiVectorKey const& positionKey = aiChannel->mPositionKeys[j];
			positionKeys.push_back(VectorKey{ static_cast<float>(positionKey.mTime), toGlmVec3(positionKey.mValue) });
		}

		// 2. rotation
		std::vector<QuatKey> rotationKeys;
		rotationKeys.reserve(aiChannel->mNumRotationKeys);

		for (unsigned j = 0; j < aiChannel->mNumRotationKeys; j++) {
			aiQuatKey const& rotationKey = aiChannel->mRotationKeys[j];
			rotationKeys.push_back(QuatKey{ static_cast<float>(rotationKey.mTime), toGlmQuat(rotationKey.mValue) });
		}

		// 3. scaling
		std::vector<VectorKey> scalingKeys;
		scalingKeys.reserve(aiChannel->mNumScalingKeys);

		for (unsigned j = 0; j < aiChannel->mNumScalingKeys; j++) {
			aiVectorKey const& scalingKey = aiChannel->mScalingKeys[j];
			scalingKeys.push_back(VectorKey{ static_cast<float>(scalingKey.mTime), toGlmVec3(scalingKey.mValue) });
		}

		AnimationChannel channel {
			std::move(positionKeys),
			std::move(rotationKeys),
			std::move(scalingKeys)
		};

		animationChannel.insert({ aiChannel->mNodeName.C_Str(), std::move(channel) });
	}
	
	return {
		animation->mName.C_Str(),
		static_cast<float>(animation->mDuration),
		static_cast<float>(animation->mTicksPerSecond),
		std::move(animationChannel)
	};
}

BoneIndex ModelLoader::findParentBone(aiNode const* parentNode) {
	if (parentNode) {
		auto parentBoneIterator = boneNameToIndex.find(parentNode->mName.C_Str());

		if (parentBoneIterator != boneNameToIndex.end()) {
			return parentBoneIterator->second;
		}
		else {
			return findParentBone(parentNode->mParent);
		}
	}

	return NO_BONE;
}
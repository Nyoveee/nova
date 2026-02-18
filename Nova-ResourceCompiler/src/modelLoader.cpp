#include <Assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include "vertex.h"

#include "modelLoader.h"
#include "Logger.h"
#include "Library/meshoptimizer.h"

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

	bool doesModelHaveBones(const aiScene* scene) {
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[i];
			if (mesh->mNumBones > 0) {
				return true; // Found at least one mesh with bones
			}
		}

		return false; // No bones found in any mesh
	}
}

std::optional<ModelData> ModelLoader::loadModel(std::string const& filepath, float scale, std::vector<BoneIndex> sockets) {
	// --------------------------------------------------------------------
	// 1. We prepare assimp to load the model..
	// --------------------------------------------------------------------
	constexpr unsigned int PostProcessingFlags {
		aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	};

	Assimp::Importer importer;

	aiScene const* scene = importer.ReadFile(filepath, PostProcessingFlags);

	if (!scene || !scene->mRootNode) {
		Logger::error("Error when importing model with filepath {} : {}", filepath, importer.GetErrorString());
		return std::nullopt;
	}

	// --------------------------------------------------------------------
	// 2. Initialisation and preparation.. We utilize class data members to share data across functions..
	// --------------------------------------------------------------------

	bones.clear();
	boneNameToIndex.clear();
	meshNameToIndex.clear();
	materialNames.clear();
	maxDimension = 0.f;
	maxBound = glm::vec3{ std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
	minBound = glm::vec3{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	hasBones = doesModelHaveBones(scene);

	// --------------------------------------------------------------------
	// 3. We process the node hierarchy recursively, and populate mesh data with vertex attributes..
	// This also handles vertex weights if the mesh has bgones..
	// --------------------------------------------------------------------
	std::vector<MeshData> meshesData;
	meshesData.reserve(scene->mNumMeshes);
	
	// We process the node hierarchy, and load meshes accordingly..
	processNodeHierarchy(scene, meshesData, scene->mRootNode, glm::mat4{ 1.f });

	// --------------------------------------------------------------------
	// 4. We now process bones, skeletons and animation data..
	// Processing animation requires converting from assimp format to our own Animation data structure..
	// Processing skeleton requires traversing the node hierarchy and store both the bone hierarchy and node hierarchy,
	// in our own data structures..
	// --------------------------------------------------------------------
	
	// Process animation data..
	std::vector<Animation> animations;
	std::optional<Skeleton> skeletonOpt;

	for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
		animations.push_back(processAnimation(scene->mAnimations[i]));
	}

	// Process node hierarchy.. (if this model has bones..)
	if (bones.size()) {
		Skeleton skeleton {};
		skeleton.bones = std::move(bones);
		skeleton.sockets = std::move(sockets);

		// start recursive processing..
		processBoneNodeHierarchy(skeleton, scene->mRootNode, NO_NODE);
		skeletonOpt = std::move(skeleton);
	}

	// --------------------------------------------------------------------
	// 5. Calculate additional meta data..
	// --------------------------------------------------------------------
	
	// Calculate center point and extents..
	glm::vec3 centerPoint = (maxBound + minBound) / 2.f;
	glm::vec3 extent = centerPoint - minBound;

	// --------------------------------------------------------------------
	// 6. We perform mesh optimization, full details can be found in the library of mesh optimzer..
	// --------------------------------------------------------------------
	
	for (MeshData& mesh : meshesData) {
		optimizeMesh(mesh);
	}

	// --------------------------------------------------------------------
	// 7. Because mesh optimizer requires the vertex attributes to be combined into one structure, we now convert
	// our MeshData struct to Mesh struct, by flattening combined vertex attribute from an array of structures to a structure of arrays..
	// 
	// When processing our MeshData struct, we may also encounter situations where the indices is zero (probably cause non triangle primitives are skipped).
	// Therefore, we remove the mesh to save draw call.
	// Consequently, we need to remap the mesh index of every model node in the skeleton to reflect the new corrected mesh index, due to the change in size of the mesh vector.
	// --------------------------------------------------------------------
	std::vector<Mesh> meshes;
	meshes.reserve(meshesData.size());

	std::unordered_map<MeshIndex, MeshIndex> meshIndexRemap;

	int validMeshIndex = 0;

	for (int i = 0; i < meshesData.size(); ++i) {
		MeshData& meshData = meshesData[i];

		// we dont even wanna draw these meshes if they have no indices..
		if (meshData.indices.empty()) {
			Logger::warn("Empty mesh found?");
			continue;
		}

		// we update mesh index mapping.. (invalid mesh have their entry removed)
		meshIndexRemap[i] = validMeshIndex;
		validMeshIndex++;

		std::vector<glm::vec3> positions;
		positions.reserve(meshData.combinedVertexAttributes.size());

		std::vector<glm::vec2> textureCoordinates;
		textureCoordinates.reserve(meshData.combinedVertexAttributes.size());

		std::vector<glm::vec3> normals;
		normals.reserve(meshData.combinedVertexAttributes.size());

		std::vector<glm::vec3> tangents;
		tangents.reserve(meshData.combinedVertexAttributes.size());

		std::vector<VertexWeight> vertexWeights;
		vertexWeights.reserve(meshData.combinedVertexAttributes.size());

		for (CombinedVertexAttribute const& vertexAttribute : meshData.combinedVertexAttributes) {
			positions.push_back(vertexAttribute.position);
			textureCoordinates.push_back(vertexAttribute.textureCoordinate);
			normals.push_back(vertexAttribute.normal);
			tangents.push_back(vertexAttribute.tangent);

			// vertex attribute is not all no bones..
			if (vertexAttribute.vertexWeight.boneIndices[0] != NO_BONE_INDEX) {
				vertexWeights.push_back(vertexAttribute.vertexWeight);
			}
		}

		meshes.push_back(
			Mesh{
				.name				= std::move(meshData.name),
				.positions			= std::move(positions),
				.textureCoordinates = std::move(textureCoordinates),
				.normals			= std::move(normals),
				.tangents			= std::move(tangents),
				.indices			= std::move(meshData.indices),
				.materialIndex		= meshData.materialIndex,
				.numOfTriangles		= meshData.numOfTriangles,
				.vertexWeights		= std::move(vertexWeights)
			}
		);
	}

	// We then remap the mesh index in the model nodes..
	if (skeletonOpt) {
		remapMeshIndex(meshIndexRemap, skeletonOpt.value(), skeletonOpt.value().nodes[skeletonOpt.value().rootNode]);
	}

	// We are done :)
	ModelData modelData{ 
		.meshes			= std::move(meshes),
		.materialNames	= std::move(materialNames), 
		.skeleton		= std::move(skeletonOpt), 
		.animations		= std::move(animations), 
		.maxDimension	= maxDimension * scale, 
		.scale			= scale, 
		.maxBound		= maxBound * scale, 
		.minBound		= minBound * scale, 
		.center			= centerPoint * scale, 
		.extents		= extent * scale
	};

	return modelData;
}


MeshData ModelLoader::processMesh(aiMesh const* mesh, aiScene const* scene, glm::mat4x4 const& globalTransformationMatrix) {
	// ==================================== 
	// Getting vertex attributes..
	// 1. position
	// 2. texture coordinates
	// 3. normal
	// 4. tangent
	// ==================================== 
	std::vector<CombinedVertexAttribute> vertexAttributes;
	vertexAttributes.reserve(mesh->mNumVertices);

	glm::mat3x3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(globalTransformationMatrix)));

	// Getting vertex attributes..
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		glm::vec3 position;

		// 1. Position
		if (hasBones) {
			position = glm::vec3{ glm::vec4{ toGlmVec3(mesh->mVertices[i]), 1.f } };
		}
		else {
			position = glm::vec3{ globalTransformationMatrix * glm::vec4{ toGlmVec3(mesh->mVertices[i]), 1.f } };
		}

		// calculating max dimension
		if (std::abs(position.x) > maxDimension || std::abs(position.y) > maxDimension || std::abs(position.z) > maxDimension) {
			maxDimension = std::max(std::max(std::abs(position.x), std::abs(position.y)), std::abs(position.z));
		}

		// calculating bounds..
		maxBound = glm::max(position, maxBound);
		minBound = glm::min(position, minBound);

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
			if (mesh->mNumBones) {
				normal = toGlmVec3(mesh->mNormals[i]);
			}
			else {
				normal = normalMatrix * toGlmVec3(mesh->mNormals[i]);
			}
		}
		else {
			normal = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		// 4. Tangents
		glm::vec3 tangent;

		if (mesh->mTangents) {
			if (mesh->mNumBones) {
				tangent = toGlmVec3(mesh->mTangents[i]);
			}
			else {
				tangent = normalMatrix * toGlmVec3(mesh->mTangents[i]);
			}
		}
		else {
			tangent = glm::vec3{ 0.0f, 0.0f, 0.f };
		}

		vertexAttributes.push_back(CombinedVertexAttribute{
			.position			= position,
			.textureCoordinate	= textureCoords,
			.normal				= normal,
			.tangent			= tangent
		});
	}

	// ==================================== 
	// Getting indices for IBO (glDrawElement). 
	// Vertices were loaded in order.
	// ====================================
	
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
	
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			if (face.mNumIndices != 3) {
				Logger::warn("Number of indices not multiple to 3?");
			}
			else {
				indices.push_back(face.mIndices[j]);
			}
		}
	}

	// ==================================== 
	// Getting material information..
	// ==================================== 
	unsigned int materialIndex = 0;

	if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::string materialName = material->GetName().C_Str();

		auto iterator = std::ranges::find(materialNames, materialName);

		if (iterator != materialNames.end()) {
			materialIndex = static_cast<unsigned int>(std::distance(materialNames.begin(), iterator));
		}
		else {
			materialIndex = static_cast<unsigned int>(materialNames.size());
			materialNames.push_back(std::move(materialName));
		}
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
	std::vector<IntermediaryVertexWeight> intermediaryVertexWeights;

	// it is a skinned mesh.
	if (mesh->mNumBones) {
		intermediaryVertexWeights.resize(mesh->mNumVertices);
		bones.reserve(bones.size() + mesh->mNumBones);

		BoneIndex boneIndex;

		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			aiBone const* bone = mesh->mBones[i];

			std::string boneName = bone->mName.C_Str();

			glm::mat4x4 offsetMatrix = toGlmMat4(bone->mOffsetMatrix);

			// We first see if this bone exist..
			if (auto iterator = boneNameToIndex.find(boneName); iterator != boneNameToIndex.end()) {
				boneIndex = iterator->second;
			}
			// bone doesn't exist, we use a new index.
			else {
				boneIndex = static_cast<BoneIndex>(bones.size());
				bones.push_back({ boneName, std::move(offsetMatrix) });
				boneNameToIndex.insert({ boneName, boneIndex });
			}

			for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
				auto aiVertexWeight = bone->mWeights[j];
				intermediaryVertexWeights[aiVertexWeight.mVertexId].addBone(boneIndex, aiVertexWeight.mWeight);
			}
		}

		// Re-normalize bone weights if need be.
		for (auto&& intermediaryVertexWeight : intermediaryVertexWeights) {
			intermediaryVertexWeight.normalizeAndSetBoneWeight();
		}

		// both resized to mesh->mNumVertices..
		assert(intermediaryVertexWeights.size() == vertexAttributes.size() && "Vertex attribute having a different size with vertex weights?");

		// Copy vertex weight data to combined vertex attribute data..
		for (int i = 0; i < vertexAttributes.size(); ++i) {
			vertexAttributes[i].vertexWeight = VertexWeight{ intermediaryVertexWeights[i].boneIndices, intermediaryVertexWeights[i].weights };
		}
	}

	return MeshData { 
		.name						= mesh->mName.C_Str(),
		.indices					= std::move(indices), 
		.materialIndex				= materialIndex,
		.numOfTriangles				= static_cast<int>(mesh->mNumFaces), 
		.combinedVertexAttributes	= std::move(vertexAttributes)
	};
}

void ModelLoader::processBoneNodeHierarchy(Skeleton& skeleton, aiNode const* node, ModelNodeIndex parentNodeIndex) {
	// process node hierarchy..

	ModelNode::Type modelNodeType = ModelNode::Type::None;
	BoneIndex nodeBoneIndex = NO_BONE;
	MeshIndex meshIndex = NOT_A_MESH;

	auto boneIterator = boneNameToIndex.find(node->mName.C_Str());
	auto meshIterator = meshNameToIndex.find(node->mName.C_Str());

	// ------------------------------------------------
	// we verify if the current node is a bone...
	if (boneIterator != boneNameToIndex.end()) {
		// we perform bone specific data handling..
		auto&& [_, boneIndex] = *boneIterator;
		modelNodeType = ModelNode::Type::Bone;
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
	// ------------------------------------------------
	// we verify if the current node is a mesh...
	else if (meshIterator != meshNameToIndex.end()) {
		modelNodeType = ModelNode::Type::Mesh;
		meshIndex = meshIterator->second;
	}
	
	// we handle node hierarchy here..
	ModelNodeIndex modelNodeIndex = static_cast<ModelNodeIndex>(skeleton.nodes.size());	// get an appropriate index for node.
	
	ModelNode modelNode {
		node->mName.C_Str(),
		toGlmMat4(node->mTransformation),
		modelNodeType,
		nodeBoneIndex,
		meshIndex
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
		processBoneNodeHierarchy(skeleton, childNode, modelNodeIndex);
	}
}

void ModelLoader::processNodeHierarchy(aiScene const* scene, std::vector<MeshData>& meshesData, aiNode const* node, glm::mat4x4 const& globalTransformationMatrix) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		meshNameToIndex.insert({ mesh->mName.C_Str(), meshesData.size() });
		meshesData.push_back(processMesh(mesh, scene, globalTransformationMatrix));
	}

	// recurse downwards.
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		aiNode const* childNode = node->mChildren[i];
		processNodeHierarchy(scene, meshesData, childNode, toGlmMat4(childNode->mTransformation) * globalTransformationMatrix);
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

void ModelLoader::printMatrix(glm::mat4x4 const matrix) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::cout << std::left << std::setw(6) << std::fixed << std::setprecision(2) << matrix[j][i] << " "; // Column-major access
		}
		std::cout << std::endl;
	}
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

void ModelLoader::optimizeMesh(MeshData& mesh) {
	// https://github.com/zeux/meshoptimizer
	// We create a temporary remap table, remapping our original vertices to a new set of vertices..
	std::vector<unsigned int> remap(mesh.combinedVertexAttributes.size());

	// if we have a valid index buffer, use that size. else use vertex buffer size.
	int indexCount = mesh.indices.data() ? mesh.indices.size() : mesh.combinedVertexAttributes.size();

	// We utilize the library to perform index remapping, and removing redundant vertices..
	// This also preps our data for the upcoming optimization.. so its just safer to do this first.
	size_t vertexCount = meshopt_generateVertexRemap(
		remap.data(), 
		mesh.indices.data(), 
		indexCount,
		mesh.combinedVertexAttributes.data(),
		mesh.combinedVertexAttributes.size(),
		sizeof(CombinedVertexAttribute)
	);

	if (vertexCount != mesh.combinedVertexAttributes.size()) {
		Logger::debug("Mesh optimizer reduced unique vertex count from {} to {}.", mesh.combinedVertexAttributes.size(), vertexCount);
	}

	// We now perform the remapping..
	// remapping index buffer..
	std::vector<unsigned int> indices(mesh.indices.size());
	meshopt_remapIndexBuffer(indices.data(), mesh.indices.data(), mesh.indices.size(), remap.data());
		
	// remapping all vertex attributes..
	std::vector<CombinedVertexAttribute> remappedVertexAttribute(vertexCount);
	meshopt_remapVertexBuffer(remappedVertexAttribute.data(), mesh.combinedVertexAttributes.data(), mesh.combinedVertexAttributes.size(), sizeof(CombinedVertexAttribute), remap.data());

	// We perform vertex cache optimization, reordering the indices such that it maximises locality..
	meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertexCount);

	// We optimize for overdraw.. this will degrade cache locality, so we use the recommended constant of 1.05f indicating we allow degrading of cache locality up to 5%..
	meshopt_optimizeOverdraw(indices.data(), indices.data(), indices.size(), &remappedVertexAttribute[0].position.x, vertexCount, sizeof(CombinedVertexAttribute), 1.05f);

	// We optimize for vertex fetch locality now, reordering the vertex.. (consequently the indices have to be reordered as well)..
	meshopt_optimizeVertexFetch(remappedVertexAttribute.data(), indices.data(), indices.size(), remappedVertexAttribute.data(), vertexCount, sizeof(CombinedVertexAttribute));

	// update our vertex attributes and indices..
	mesh.combinedVertexAttributes = std::move(remappedVertexAttribute);
	mesh.indices = std::move(indices);
}

void ModelLoader::remapMeshIndex(std::unordered_map<MeshIndex, MeshIndex> const& meshIndexRemap, Skeleton& skeleton, ModelNode& modelNode) {
	if (modelNode.nodeType == ModelNode::Type::Mesh) {
		// this node is a mesh.. let's remap it..
		auto iterator = meshIndexRemap.find(modelNode.meshIndex);

		// The mesh this node is pointing to is removed.
		if (iterator == meshIndexRemap.end()) {
			modelNode.nodeType = ModelNode::Type::None;
			modelNode.meshIndex = NOT_A_MESH;
		}
		else {
			modelNode.meshIndex = iterator->second;
		}
	}

	// recurse downwards.
	for (unsigned int i = 0; i < modelNode.nodeChildrens.size(); ++i) {
		ModelNode& childNode = skeleton.nodes[modelNode.nodeChildrens[i]];
		remapMeshIndex(meshIndexRemap, skeleton, childNode);
	}
}

#pragma once

#include <limits>
#include <vector>

#undef max

using BoneIndex			= unsigned short;
using ModelNodeIndex	= unsigned int;

constexpr BoneIndex		 NO_BONE			= std::numeric_limits<BoneIndex>::max();
constexpr ModelNodeIndex NO_NODE			= std::numeric_limits<ModelNodeIndex>::max();

constexpr int			 NO_BONE_INDEX		= -1;
constexpr int			 MAX_BONE_INFLUENCE = 4;

/*
	A bone is a specific type of model node that contains the offset matrix.

	Vertices are influences by the bone, and therefore it provides this offset matrix to
	transform vertices from model space to bone space.
*/
struct Bone {
	std::string name;
	glm::mat4x4 offsetMatrix;

	// containing data related to the bone hirerarchy.
	BoneIndex parentBone = NO_BONE;
	std::vector<BoneIndex> boneChildrens{};

	REFLECTABLE(
		name,
		offsetMatrix,
		parentBone,
		boneChildrens
	)
};

/*
	Model Nodes represents an Assimp node. It is part of the node hirerachy, and contain
	important data like name, transformation matrix.

	A ModelNode may be a bone. In that case, the isBone flag is set to true, and it contains an index to query the bone
	hierarchy to retrieve the corresponding bone data. 
*/
struct ModelNode {
	std::string name;
	glm::mat4x4 transformationMatrix;

	// indicate if this node is actually a bone.
	bool isBone			= false;
	BoneIndex boneIndex = NO_BONE;	// if this node is a bone, contains the index to retrieve it's corresponding bone data.

	// containing data related to the node hirerarchy.
	ModelNodeIndex parentNode = NO_NODE;
	std::vector<ModelNodeIndex> nodeChildrens{};

	REFLECTABLE(
		name,
		transformationMatrix,
		isBone,
		boneIndex,
		parentNode,
		nodeChildrens
	)
};


// Vertex Weight is a vertex attribute that maps vertices to the bones that influences it.
struct VertexWeight {
	std::array<int,		MAX_BONE_INFLUENCE>		boneIndices	{ NO_BONE_INDEX, NO_BONE_INDEX, NO_BONE_INDEX, NO_BONE_INDEX };
	std::array<float,	MAX_BONE_INFLUENCE>		weights		{ 0.f, 0.f, 0.f, 0.f };

	unsigned int numOfBones = 0;

	REFLECTABLE(
		boneIndices,
		weights
	)

	bool addBone(BoneIndex boneIndex, float weight) {
		if (numOfBones >= MAX_BONE_INFLUENCE) {
			return false;
		}

		boneIndices[numOfBones] = boneIndex;
		weights[numOfBones] = weight;

		++numOfBones;
		return true;
	}
};

/*
	The Skeleton class stores 2 hierarchy 
		- Node hierarchy
		- Bone only hierarchy

	Node hierarchy contains every single node that assimp, including intermediary nodes that are not actually bone.
	It is important to keep this hierarchy because intermediary nodes are still involved in animation, affecting the transformation chain
	even if they don't affect vertices directly.

	Bone only hierarchy contains only the bones. This is good for UI purposes, and so vertices can mapped to the which bones is affecting 
	via BoneIndex.
*/
struct Skeleton {
	// Node hierarchy
	std::vector<ModelNode> nodes {};
	ModelNodeIndex rootNode		 = NO_NODE;

	// Bone hierarchy
	std::vector<Bone> bones		 {};
	BoneIndex rootBone			 = NO_BONE;

	REFLECTABLE(
		nodes,
		rootNode,
		bones,
		rootBone
	)
};
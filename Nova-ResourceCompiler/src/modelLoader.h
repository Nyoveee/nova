#pragma once
#include <string>
#include <optional>

#include "model.h"
#include "reflection.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiNode;
struct aiAnimation;

class ModelLoader {
public:
	static std::optional<ModelData> loadModel(std::string const& filepath);

private:
	static Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension);
	static void processNodeHierarchy(Skeleton& skeleton, aiNode const* node, ModelNodeIndex parentNodeIndex);
	static void printBone(BoneIndex boneIndex, unsigned int padding);
	
	static Animation processAnimation(aiAnimation const* assimpAnimation);

	static BoneIndex findParentBone(aiNode const* parentNode);		

private:
	inline static std::vector<Bone> bones {};
	inline static std::unordered_map<std::string, BoneIndex> boneNameToIndex {};
};
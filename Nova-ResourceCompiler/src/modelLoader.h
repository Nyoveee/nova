#pragma once
#include <string>
#include <optional>

#include "model.h"
#include "reflection.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiNode;

class ModelLoader {
public:
	static std::optional<ModelData> loadModel(std::string const& filepath);

private:
	static Mesh processMesh(aiMesh const* mesh, aiScene const* scene, float& maxDimension, unsigned int vertexOffset);
	static void processNodeHierarchy(aiNode const* node);
	static void printBone(BoneIndex boneIndex, unsigned int padding);
	
	static BoneIndex findParentBone(aiNode const* parentNode);

private:
	inline static std::vector<Bone> bones {};
	inline static BoneIndex rootBone {};
	inline static std::unordered_map<std::string, BoneIndex> boneNameToIndex {};
};
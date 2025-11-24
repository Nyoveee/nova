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
	static std::optional<ModelData> loadModel(std::string const& filepath, float scale);

private:
	static Mesh processMesh(aiMesh const* mesh, aiScene const* scene, glm::mat4x4 const& globalTransformationMatrix);
	static void processBoneNodeHierarchy(Skeleton& skeleton, aiNode const* node, ModelNodeIndex parentNodeIndex);
	static void processNodeHierarchy(aiScene const* scene, std::vector<Mesh>& meshes, aiNode const* node, glm::mat4x4 const& globalTransformationMatrix);

	static void printBone(BoneIndex boneIndex, unsigned int padding);
	static void printMatrix(glm::mat4x4 const matrix);

	static Animation processAnimation(aiAnimation const* assimpAnimation);

	static BoneIndex findParentBone(aiNode const* parentNode);		

private:
	inline static float maxDimension;
	inline static std::vector<MaterialName> materialNames;
	inline static std::vector<Bone> bones {};

#if false
	inline static std::vector<VertexWeight> vertexWeights {};
	inline static std::unordered_map<std::string, int> meshVertexBaseOffset = {};
	inline static int vertexOffset = 0;
#endif

	inline static std::unordered_map<std::string, BoneIndex> boneNameToIndex {};
};
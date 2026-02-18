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

// intermediary mesh data..
struct MeshData {
	std::string name;
	std::vector<unsigned int> indices;
	unsigned int materialIndex;			// holds an index to the std::vector of material names in the model class.
	int numOfTriangles;

	std::vector<CombinedVertexAttribute> combinedVertexAttributes;
};

// a working vertex weight used by the model..
struct IntermediaryVertexWeight {
	std::array<int, MAX_BONE_INFLUENCE>		boneIndices{ NO_BONE_INDEX, NO_BONE_INDEX, NO_BONE_INDEX, NO_BONE_INDEX };
	std::array<float, MAX_BONE_INFLUENCE>	weights{ 0.f, 0.f, 0.f, 0.f };

	// when we add vertex weight, we add to a temporary copy.
	void addBone(BoneIndex boneIndex, float weight) {
		temporaryBoneWeights.push_back({ boneIndex, weight });
	}

	// this will trim the bone weights in the temporary vector to 4 and normalize if needed.
	void normalizeAndSetBoneWeight() {
		if (temporaryBoneWeights.size() > 4) {
			Logger::warn("Too many vertex weight, re-normalising..");
			std::ranges::sort(temporaryBoneWeights, [&](auto&& lhs, auto&& rhs) {
				return lhs.second > rhs.second;
			});

			temporaryBoneWeights.resize(4);

			// get the new sum.. then normalise..
			float totalSum = std::accumulate(temporaryBoneWeights.begin(), temporaryBoneWeights.end(), 0.f, [&](float sum, auto&& boneToVertexWeight) {
				return sum += boneToVertexWeight.second;
			});

			std::for_each(temporaryBoneWeights.begin(), temporaryBoneWeights.end(), [&](auto&& boneToVertexWeight) {
				boneToVertexWeight.second /= totalSum;
			});
		}

		unsigned int numOfBones = 0;

		for (auto&& [index, weight] : temporaryBoneWeights) {
			boneIndices[numOfBones] = index;
			weights[numOfBones] = weight;
			++numOfBones;
		}
	}

	// temporarily store bone weights for normalization later.. a vector because of the possibility of more than one..
	std::vector<std::pair<int, float>> temporaryBoneWeights;
};

class ModelLoader {
public:
	static std::optional<ModelData> loadModel(std::string const& filepath, float scale, std::vector<BoneIndex> sockets);

private:
	static MeshData processMesh(aiMesh const* mesh, aiScene const* scene, glm::mat4x4 const& globalTransformationMatrix);
	static void processBoneNodeHierarchy(Skeleton& skeleton, aiNode const* node, ModelNodeIndex parentNodeIndex);
	static void processNodeHierarchy(aiScene const* scene, std::vector<MeshData>& meshesData, aiNode const* node, glm::mat4x4 const& globalTransformationMatrix);

	static void printBone(BoneIndex boneIndex, unsigned int padding);
	static void printMatrix(glm::mat4x4 const matrix);

	static Animation processAnimation(aiAnimation const* assimpAnimation);

	static BoneIndex findParentBone(aiNode const* parentNode);

	// we use meshoptimizer to optimize mesh data..
	static void optimizeMesh(MeshData& mesh);

	static void remapMeshIndex(std::unordered_map<MeshIndex, MeshIndex> const& meshIndexRemap, Skeleton& skeleton, ModelNode& modelNode);

private:
	inline static float maxDimension;
	inline static glm::vec3 maxBound;
	inline static glm::vec3 minBound;

	inline static std::vector<MaterialName> materialNames;
	inline static std::vector<Bone> bones {};

	inline static bool hasBones = false;

#if false
	inline static std::vector<VertexWeight> vertexWeights {};
	inline static std::unordered_map<std::string, int> meshVertexBaseOffset = {};
	inline static int vertexOffset = 0;
#endif

	inline static std::unordered_map<std::string, BoneIndex> boneNameToIndex {};
	inline static std::unordered_map<std::string, MeshIndex> meshNameToIndex {};
};
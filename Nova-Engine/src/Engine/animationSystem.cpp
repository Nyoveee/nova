#include "animationSystem.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

AnimationSystem::AnimationSystem(Engine& p_engine) : 
	engine { p_engine },
	resourceManager	{ p_engine.resourceManager }
{}

void AnimationSystem::update([[maybe_unused]] float dt) {
	entt::registry& registry = engine.ecs.registry;

	for (auto&& [entityId, transform, skinnedMeshRenderer] : registry.view<Transform, SkinnedMeshRenderer>().each()) {
		auto&& [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);
		
		if (!model || !model->bones.size()) {
			continue;
		}

		skinnedMeshRenderer.bonesFinalMatrices.clear();
		skinnedMeshRenderer.bonesFinalMatrices.resize(model->bones.size());

		// we find the root bone first, and recursively calculate the final transformation matrix down...
		BoneIndex rootBone = model->rootBone;

		calculateFinalMatrix(rootBone, model->bones[rootBone].transformationMatrix, model->bones, skinnedMeshRenderer);
	}
}

// different transformation names
// -> offset matrix			: maps from model space to local bone space.
// -> transformation		: maps from local bone space to parent bone space. (chained w/ all parent node that is not a bone)
// -> globalTransformation	: maps from local bone space to model space. (if bind pose, the exact inverse of offset matrix. with animation, it will change.)
// -> finalTransformation	: globalTransformation * offset matrix. we essientially map from local, to bone, back to local.
void AnimationSystem::calculateFinalMatrix(BoneIndex boneIndex, glm::mat4x4 const& globalTransformationMatrix, std::vector<Bone> const& bones, SkinnedMeshRenderer& skinnedMeshRenderer) {
	Bone const& bone = bones[boneIndex];

	// calculate final transformation.
	glm::mat4x4 finalTransformation = globalTransformationMatrix * bones[boneIndex].offsetMatrix;
	skinnedMeshRenderer.bonesFinalMatrices[boneIndex] = finalTransformation;
		
	// recurse downwards..
	for (BoneIndex childBoneIndex : bone.boneChildrens) {
		// calculate child's global transformation -> which is parent's global transformation * children's transformation.
		glm::mat4x4 childGlobalTransformation = globalTransformationMatrix * bones[childBoneIndex].transformationMatrix;
		calculateFinalMatrix(childBoneIndex, childGlobalTransformation, bones, skinnedMeshRenderer);
	}
}
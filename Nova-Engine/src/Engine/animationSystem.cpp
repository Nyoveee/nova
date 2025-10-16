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
		
		if (!model) {
			continue;
		}

		skinnedMeshRenderer.bonesFinalMatrices.clear();
		skinnedMeshRenderer.bonesFinalMatrices.resize(model->bones.size());

		// we find the root bone first, and recursively calculate the final transformation matrix down...
		BoneIndex rootBone = model->rootBone;

		calculateFinalMatrix(rootBone, model->bones, skinnedMeshRenderer);

		std::cout << "asd\n";
	}
}

// different transformation names
// -> offset matrix			: maps from model space to bone space.
// -> mTransformation		: maps from local space to parent space.
// -> globalTransformation	: maps from local space to model space.
// -> finalTransformation	: globalTransformation * offset matrix. we essientially map from local, to bone, back to local.
void AnimationSystem::calculateFinalMatrix(BoneIndex boneIndex, std::vector<Bone> const& bones, SkinnedMeshRenderer& skinnedMeshRenderer) {
	Bone const& bone = bones[boneIndex];

	// calculate final transformation.
	glm::mat4x4 finalTransformation = bones[boneIndex].globalTransformationMatrix * bones[boneIndex].offsetMatrix;

	// recurse downwards..
	for (BoneIndex childBoneIndex : bone.boneChildrens) {
		calculateFinalMatrix(childBoneIndex, bones, skinnedMeshRenderer);
	}
}
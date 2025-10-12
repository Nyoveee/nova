#include "animationSystem.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

namespace {
	glm::mat4x4 getFinalTransformationMatrix(BoneIndex boneIndex, SkinnedMeshRenderer& skinnedMeshRenderer, std::vector<Bone> const& bones) {
#if 0
		// final transformation matrix has already been calculated.
		if (skinnedMeshRenderer.cachedBones.count(boneIndex)) {
			return skinnedMeshRenderer.bonesFinalMatrices[boneIndex];
		}
#endif

		Bone const& bone = bones[boneIndex];

		if (bone.parentBone == NO_BONE) {
			return bone.parentTransformationMatrix;
		}

		return getFinalTransformationMatrix(bone.parentBone, skinnedMeshRenderer, bones) * bone.parentTransformationMatrix;
	}
}
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

		skinnedMeshRenderer.cachedBones.clear();
		skinnedMeshRenderer.bonesFinalMatrices.clear();
		skinnedMeshRenderer.bonesFinalMatrices.resize(model->bones.size());

		for (BoneIndex boneIndex = 0; boneIndex < model->bones.size(); boneIndex++) {
			skinnedMeshRenderer.bonesFinalMatrices.push_back(getFinalTransformationMatrix(boneIndex, skinnedMeshRenderer, model->bones));
		}
	}
}
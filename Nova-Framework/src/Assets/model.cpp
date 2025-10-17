#include "model.h"
#include "Logger.h"

Model::Model(ResourceID id, ResourceFilePath resourceFilePath, ModelData modelData) :
	Resource				{ id, std::move(resourceFilePath) },
	meshes					{ std::move(modelData.meshes) },
	materialNames			{ std::move(modelData.materialNames) },
	bones					{ std::move(modelData.bones) },
	rootBone				{ modelData.rootBone },
	animations				{ std::move(modelData.animations) },
	maxDimension			{ modelData.maxDimension }
{}

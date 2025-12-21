#include "model.h"
#include "Logger.h"

Model::Model(ResourceID id, ResourceFilePath resourceFilePath, ModelData modelData) :
	Resource				{ id, std::move(resourceFilePath) },
	meshes					{ std::move(modelData.meshes) },
	materialNames			{ std::move(modelData.materialNames) },
	skeleton				{ std::move(modelData.skeleton) },
	animations				{ std::move(modelData.animations) },
	maxDimension			{ modelData.maxDimension },
	scale					{ modelData.scale },
	maxBound				{ modelData.maxBound },
	minBound				{ modelData.minBound },
	center					{ modelData.center },
	extents					{ modelData.extents }
{}

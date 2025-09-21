#include "model.h"
#include "Logger.h"

Model::Model(ResourceID id, ResourceFilePath resourceFilePath, std::vector<Mesh> meshes, std::unordered_set<MaterialName> materialNames) :
	Resource		{ id, std::move(resourceFilePath) },
	meshes			{ std::move(meshes) },
	materialNames	{ std::move(materialNames) },
	maxDimension	{}
{}

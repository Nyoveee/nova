#include "model.h"
#include "Logger.h"

Model::Model(ResourceID id, std::vector<Mesh> meshes, std::unordered_set<MaterialName> materialNames) :
	Asset			{ id },
	meshes			{ std::move(meshes) },
	materialNames	{ std::move(materialNames) },
	maxDimension	{}
{}

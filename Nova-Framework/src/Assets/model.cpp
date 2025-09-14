#include "model.h"

Model::Model(std::string filepath) :
	Asset			{filepath},
	maxDimension	{}
{}

void Model::load() {
#if 0
	ModelLoader modelLoader;
	auto model = modelLoader.loadModel(getFilePath());

	if (!model) {
		this->loadStatus = LoadStatus::LoadingFailed;
		return;
	}

	// abusing move semantic to not make unnecesary copies ;)
	auto [meshes, materialNames, maxDimension] = std::move(model).value();
	this->meshes = std::move(meshes);
	this->materialNames = std::move(materialNames);
	this->maxDimension = maxDimension;

	// remember to set loading to Loaded to inform the asset manager that you are done loading!
	this->loadStatus = LoadStatus::Loaded;
	//TracyAlloc(this, sizeof(*this));
#endif
	loadStatus = LoadStatus::LoadingFailed;
}

void Model::unload() {
	meshes.clear();
	materialNames.clear();
	//TracyFree(this);
}

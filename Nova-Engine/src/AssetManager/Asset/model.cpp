#include "model.h"
#include "AssetManager/modelLoader.h"

Model::Model(std::string filepath) :
	Asset {filepath}
{}

void Model::load() {
	ModelLoader modelLoader;
	Asset::hasLoaded = modelLoader.loadModel(*this);
}

void Model::unload() {
	meshes.clear();
	materialNames.clear();
	Asset::hasLoaded = false;
}

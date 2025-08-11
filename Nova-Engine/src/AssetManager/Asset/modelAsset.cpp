#include "modelAsset.h"
#include "AssetManager/modelLoader.h"

ModelAsset::ModelAsset(std::string filepath) :
	Asset {filepath}
{}

void ModelAsset::load() {
	ModelLoader modelLoader;
	
	std::optional<Model> loadedModel = modelLoader.loadModel(getFilePath());

	if (loadedModel) {
		Asset::hasLoaded = true;
		model = std::move(loadedModel.value());
	}
}

void ModelAsset::unload() {
	model.clear();
	Asset::hasLoaded = false;
}

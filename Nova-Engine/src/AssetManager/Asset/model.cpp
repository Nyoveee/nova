#include "model.h"
#include "AssetManager/modelLoader.h"
#include "assetManager.h"

Model::Model(std::string filepath) :
	Asset			{filepath},
	maxDimension	{}
{}

void Model::load(AssetManager& assetManager) {
	assetManager.threadPool.detach_task([&]() {
		ModelLoader modelLoader;
		auto model = modelLoader.loadModel(getFilePath());

		// submits a callback the finish model 
		// moves the model data into the functor, because the lifetime of functor
		// exceeds that of the model.
		// 
		// NOTE!:
		// there's actually a slight danger here, because `this` may be dangling
		// in practice because we always store the assets in the unordered_map, and we populate the unordered_map at the start without touching it,
		// and we never move the assets, this pointer should be fine
		// @TODO: find a better implementation that guarantees object validation. perhaps using AssetID?
		assetManager.submitCallback([model = std::move(model), this]() {
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
			TracyAlloc(this, sizeof(*this));
		});
	});
}

void Model::unload() {
	meshes.clear();
	materialNames.clear();
	TracyFree(this);
}

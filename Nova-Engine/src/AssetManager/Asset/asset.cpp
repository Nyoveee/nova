#include "asset.h"

//namespace {
//	constexpr inline AssetID INVALID_ID = static_cast<AssetID>(std::numeric_limits<int>::max());
//}

Asset::Asset(std::string filepath) : filepath{ filepath }, name{}, id{}, loadStatus{ LoadStatus::NotLoaded } {}
Asset::~Asset() {}

std::string const& Asset::getFilePath() const {
	return filepath;
}

Asset::LoadStatus Asset::getLoadStatus() const {
	return loadStatus;
}

bool Asset::isLoaded() const {
	return loadStatus == LoadStatus::Loaded;
}

void Asset::toLoad(AssetManager& assetManager) {
	loadStatus = LoadStatus::Loading;
	load(assetManager);
}

void Asset::toUnload() {
	unload();
	loadStatus = LoadStatus::NotLoaded;
}

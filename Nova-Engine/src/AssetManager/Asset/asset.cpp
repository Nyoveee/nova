#include "asset.h"

//namespace {
//	constexpr inline AssetID INVALID_ID = static_cast<AssetID>(std::numeric_limits<int>::max());
//}

Asset::Asset(std::string filepath) : filepath{ filepath }, name{}, id{}, hasLoaded{} {}
Asset::~Asset() {}

std::string const& Asset::getFilePath() const {
	return filepath;
}

bool Asset::isLoaded() const {
	return hasLoaded;
}

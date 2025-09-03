#pragma once

// dont worry about the intellisense error, VS2022 doesnt know when it is getting included.

class AssetManager;

using SerialiseAssetFunctorID = std::size_t;

struct SerialiseAsset {
public:
	virtual ~SerialiseAsset() = 0 {};
	virtual void operator()(Asset& asset, AssetManager& assetManager) const = 0;
};

template <ValidAsset T>
struct SerialiseAssetFunctor : public SerialiseAsset {
	void operator()(Asset& asset, AssetManager& assetManager) const final;
};
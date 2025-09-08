#pragma once

// contains functors to serialise each type of assets

class AssetManager;

struct SerialiseMetaData {
public:
	virtual ~SerialiseMetaData() = 0 {};
	virtual void operator()(Asset& asset, AssetManager& assetManager) const = 0;
};

template <ValidAsset T>
struct SerialiseMetaDataFunctor : public SerialiseMetaData {
	void operator()(Asset& asset, AssetManager& assetManager) const final;
};

struct SerialiseAsset {
public:
	virtual ~SerialiseAsset() = 0 {};
	virtual void operator()(Asset& asset, AssetManager& assetManager) const = 0;
};

template <ValidAsset T>
struct SerialiseAssetFunctor : public SerialiseAsset {
	void operator()(Asset& asset, AssetManager& assetManager) const final;
};
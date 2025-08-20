#pragma once

#include <string>

#include "Libraries/type_alias.h"
#include "export.h"

// Unfortunately, copy-and-swap idiom doesn't work well with base abstract classes.
class Asset {
public:
	DLL_API Asset(std::string filepath);

	DLL_API virtual ~Asset() = 0;
	DLL_API Asset(Asset const& other)				= delete;
	DLL_API Asset(Asset&& other)					= default;
	DLL_API Asset& operator=(Asset const& other)	= delete;
	DLL_API Asset& operator=(Asset&& other)			= default;

public:
	DLL_API virtual void load  () = 0;
	DLL_API virtual void unload() = 0;

	DLL_API std::string const& getFilePath() const;
	DLL_API bool isLoaded() const;

protected:
	bool hasLoaded;

public:
	std::string name;
	AssetID id;

private:
	std::string filepath;
};

// generic asset meta info that all assets have.
struct BasicAssetInfo {
	AssetID id;
	std::string filepath;
	std::string name;
};

// specific asset meta info.
// 1. each asset type will need to explicitly specialise this asset type, if they require additional info!
template <typename T> requires std::derived_from<T, Asset>
struct AssetInfo : public BasicAssetInfo {};

// creates the specific asset based on it's respective asset info.
// 2. each asset type will need to explicit specialise this function to create an instance of the asset, if they are using a explicitly
// specialised asset info!
template<typename T>
T createAsset(AssetInfo<T> const& assetInfo) { return T{ assetInfo.filepath }; }
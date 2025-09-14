#pragma once

#include <string>

#include "type_alias.h"
#include "export.h"

// Unfortunately, copy-and-swap idiom doesn't work well with base abstract classes.
class Asset {
public:
	enum class LoadStatus {
		NotLoaded,
		Loading,
		LoadingFailed,
		Loaded
	};

public:
	FRAMEWORK_DLL_API Asset(std::string filepath);

	FRAMEWORK_DLL_API virtual ~Asset() = 0;
	FRAMEWORK_DLL_API Asset(Asset const& other) = delete;
	FRAMEWORK_DLL_API Asset(Asset&& other) = default;
	FRAMEWORK_DLL_API Asset& operator=(Asset const& other) = delete;
	FRAMEWORK_DLL_API Asset& operator=(Asset&& other)			= default;

public:
	FRAMEWORK_DLL_API std::string const& getFilePath() const;
	FRAMEWORK_DLL_API LoadStatus getLoadStatus() const;
	FRAMEWORK_DLL_API bool isLoaded() const;

public:
	// we provide a public interface to these instead of the virtual functions directly
	// to properly set the load status.
	FRAMEWORK_DLL_API void toLoad();
	FRAMEWORK_DLL_API void toUnload();

protected:
	FRAMEWORK_DLL_API virtual void load  () = 0;
	FRAMEWORK_DLL_API virtual void unload() = 0;

protected:
	LoadStatus loadStatus;

public:
	std::string name;
	ResourceID id;

private:
	std::string filepath;
};

// generic asset meta info that all assets have.
struct BasicAssetInfo {
	ResourceID id;
	std::string name;
	std::string filepath;
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

// Assets stored in the asset manager merely point to these assets in file location.
// These assets could be loaded or not loaded.
// Asset Manager merely acts as a bookkeep to all possible assets in the engine.

// A valid asset must
// 1. Inherit from base class asset
// 2. Corresponding asset info inherit from BasicAssetInfo

// dont remove this concept! it's meant to save you from shooting yourself in the foot.
template <typename T>
concept ValidAsset = std::derived_from<T, Asset>&& std::derived_from<AssetInfo<T>, BasicAssetInfo>;
#pragma once

#include <string>
#include <fstream>

#include "type_alias.h"
#include "export.h"

class Asset {
public:
	FRAMEWORK_DLL_API Asset(ResourceID id) : id{ id } {};

	FRAMEWORK_DLL_API virtual ~Asset()						= 0 {};
	FRAMEWORK_DLL_API Asset(Asset const& other)				= delete;
	FRAMEWORK_DLL_API Asset(Asset&& other)					= default;
	FRAMEWORK_DLL_API Asset& operator=(Asset const& other)	= delete;
	FRAMEWORK_DLL_API Asset& operator=(Asset&& other)		= default;

public:
	ResourceID id;
};

// this contains descriptor data that points back to the original asset.
struct BasicAssetInfo {
	ResourceID id;
	std::string name;
	AssetFilePath filepath;
};

// specific asset meta info.
// 1. each asset type will need to explicitly specialise this asset type, if they require additional info!
template <typename T> requires std::derived_from<T, Asset>
struct AssetInfo : public BasicAssetInfo {};

// A valid asset must
// 1. Inherit from base class asset
// 2. Corresponding asset info inherit from BasicAssetInfo

// dont remove this concept! it's meant to save you from shooting yourself in the foot.
template <typename T>
concept ValidAsset = std::derived_from<T, Asset>&& std::derived_from<AssetInfo<T>, BasicAssetInfo>;

#define ALL_RESOURCES \
Texture, Model, CubeMap, ScriptAsset, Audio, Scene

#include "texture.h"
#include "model.h"
#include "cubemap.h"
#include "scriptAsset.h"
#include "audio.h"
#include "scene.h"
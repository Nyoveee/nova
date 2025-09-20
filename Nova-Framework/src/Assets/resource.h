#pragma once

#include <filesystem>
#include <string>
#include <fstream>

#include "type_alias.h"
#include "export.h"

class Resource {
public:
	FRAMEWORK_DLL_API Resource(ResourceID id, ResourceFilePath filePath) : _id{ id }, filePath{ filePath } {};

	FRAMEWORK_DLL_API virtual ~Resource()							= 0 {};
	FRAMEWORK_DLL_API Resource(Resource const& other)				= delete;
	FRAMEWORK_DLL_API Resource(Resource&& other)					= default;
	FRAMEWORK_DLL_API Resource& operator=(Resource const& other)	= delete;
	FRAMEWORK_DLL_API Resource& operator=(Resource&& other)			= default;

	FRAMEWORK_DLL_API ResourceID id() const { return _id; };
	FRAMEWORK_DLL_API ResourceFilePath const& getFilePath() const { return filePath; };

private:
	ResourceID _id;
	ResourceFilePath filePath;
};

// this contains descriptor data that points back to the original asset.
struct BasicAssetInfo {
	ResourceID id;
	std::string name;
	AssetFilePath filepath;
	std::chrono::milliseconds timeLastWrite;
};

// specific asset meta info.
// 1. each asset type will need to explicitly specialise this asset type, if they require additional info!
template <typename T> requires std::derived_from<T, Resource>
struct AssetInfo : public BasicAssetInfo {};

// A valid asset must
// 1. Inherit from base class asset
// 2. Corresponding asset info inherit from BasicAssetInfo

// dont remove this concept! it's meant to save you from shooting yourself in the foot.
template <typename T>
concept ValidResource = std::derived_from<T, Resource>&& std::derived_from<AssetInfo<T>, BasicAssetInfo>;

#define ALL_RESOURCES \
Texture, Model, CubeMap, ScriptAsset, Audio, Scene

#include "texture.h"
#include "model.h"
#include "cubemap.h"
#include "scriptAsset.h"
#include "audio.h"
#include "scene.h"
#pragma once

#include "resource.h"

class Audio : public Resource {
public:
	FRAMEWORK_DLL_API Audio(ResourceID id, ResourceFilePath resourceFilePath, bool is3D = false);
	FRAMEWORK_DLL_API ~Audio();

	FRAMEWORK_DLL_API Audio(Audio const& other)				= delete;
	FRAMEWORK_DLL_API Audio(Audio&& other)					= default;
	FRAMEWORK_DLL_API Audio& operator=(Audio const& other)	= delete;
	FRAMEWORK_DLL_API Audio& operator=(Audio&& other)		= default;

public:
	FRAMEWORK_DLL_API bool isAudio3D() const;

private:
	bool is3D;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!
template <>
struct AssetInfo<Audio> : public BasicAssetInfo {
	AssetInfo() = default;
	AssetInfo(BasicAssetInfo assetInfo) : BasicAssetInfo{ std::move(assetInfo) } {};

	bool is3D;
};
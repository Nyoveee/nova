#pragma once

#include "asset.h"

class Audio : public Asset {
public:
	FRAMEWORK_DLL_API Audio(ResourceID id, ResourceFilePath resourceFilePath, bool is3D = false);
	FRAMEWORK_DLL_API ~Audio();

	FRAMEWORK_DLL_API Audio(Audio const& other)				= delete;
	FRAMEWORK_DLL_API Audio(Audio&& other)					= default;
	FRAMEWORK_DLL_API Audio& operator=(Audio const& other)	= delete;
	FRAMEWORK_DLL_API Audio& operator=(Audio&& other)		= default;

public:
	FRAMEWORK_DLL_API bool isAudio3D() const;
	FRAMEWORK_DLL_API ResourceFilePath const& getFilePath() const;

private:
	bool is3D;
	ResourceFilePath resourceFilePath;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!
template <>
struct AssetInfo<Audio> : public BasicAssetInfo {
	bool is3D;
};
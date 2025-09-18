#pragma once

#include "asset.h"

class Audio : public Asset {
public:
	FRAMEWORK_DLL_API Audio(ResourceFilePath filepath, bool is3D = false);
	FRAMEWORK_DLL_API ~Audio();

	FRAMEWORK_DLL_API Audio(Audio const& other) = delete;
	FRAMEWORK_DLL_API Audio(Audio&& other) = default;
	FRAMEWORK_DLL_API Audio& operator=(Audio const& other) = delete;
	FRAMEWORK_DLL_API Audio& operator=(Audio&& other) = default;

public:
	// Leave blank as loading and unloading is done in audioSystem
	FRAMEWORK_DLL_API bool load() final;
	FRAMEWORK_DLL_API void unload() final;
	FRAMEWORK_DLL_API std::string getClassName() const;

public:
	FRAMEWORK_DLL_API bool isAudio3D() const;

private:
	bool is3D;
};

// Explicitly define an extension of the asset metadata
// Try to provide default values if possible!
template <>
struct AssetInfo<Audio> : public BasicAssetInfo {
	bool is3D;
};
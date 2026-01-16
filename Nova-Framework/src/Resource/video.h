#pragma once

#include "resource.h"

class Video : public Resource {
public:
	FRAMEWORK_DLL_API Video(ResourceID id, ResourceFilePath resourceFilePath);
	FRAMEWORK_DLL_API ~Video();

	FRAMEWORK_DLL_API Video(Video const& other)				= delete;
	FRAMEWORK_DLL_API Video(Video&& other)					= default;
	FRAMEWORK_DLL_API Video& operator=(Video const& other)	= delete;
	FRAMEWORK_DLL_API Video& operator=(Video&& other)		= default;

public:
	// Add any video-specific methods here if needed
	// For example, you might want to expose video dimensions, duration, etc.

private:
	// Video-specific data can be added here
};

// Explicitly define an extension of the asset metadata
template <>
struct AssetInfo<Video> : public BasicAssetInfo {
	// Add any video-specific metadata here if needed
	// For example: duration, resolution, codec info, etc.
};

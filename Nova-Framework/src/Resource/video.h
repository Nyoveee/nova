#pragma once

#include "resource.h"

// Forward declare plm_t (defined in pl_mpeg.h)
typedef struct plm_t plm_t;

using GLuint = unsigned int;

class Video : public Resource {
public:
	FRAMEWORK_DLL_API Video(ResourceID id, ResourceFilePath resourceFilePath);
	FRAMEWORK_DLL_API ~Video();

	FRAMEWORK_DLL_API Video(Video const& other)				= delete;
	FRAMEWORK_DLL_API Video(Video&& other)					= default;
	FRAMEWORK_DLL_API Video& operator=(Video const& other)	= delete;
	FRAMEWORK_DLL_API Video& operator=(Video&& other)		= default;

public:
	// Load/unload video from file
	FRAMEWORK_DLL_API bool load();
	FRAMEWORK_DLL_API void unload();

	// Decode next frame using delta time, returns true if frame was decoded
	FRAMEWORK_DLL_API bool decodeFrame(float dt);

	// Accessors
	FRAMEWORK_DLL_API GLuint getTextureY() const;
	FRAMEWORK_DLL_API GLuint getTextureCr() const;
	FRAMEWORK_DLL_API GLuint getTextureCb() const;
	FRAMEWORK_DLL_API int getWidth() const;
	FRAMEWORK_DLL_API int getHeight() const;
	FRAMEWORK_DLL_API bool isLoaded() const;

	// Playback control
	FRAMEWORK_DLL_API void setLoop(bool loop);
	FRAMEWORK_DLL_API bool getLoop() const;

private:
	plm_t* plm;
	GLuint videoTextureY;
	GLuint videoTextureCr;
	GLuint videoTextureCb;
	int width;
	int height;
	float timeAccumulator;
	bool looping;
};

// Explicitly define an extension of the asset metadata
template <>
struct AssetInfo<Video> : public BasicAssetInfo {
};

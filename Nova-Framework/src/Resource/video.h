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
	// Decode next frame using delta time
	FRAMEWORK_DLL_API void decodeFrame(float& timeAccumulator);
	FRAMEWORK_DLL_API bool decodingFinished();

	// Accessors
	FRAMEWORK_DLL_API plm_t* const getPLM() const;
	FRAMEWORK_DLL_API GLuint getTextureY() const;
	FRAMEWORK_DLL_API GLuint getTextureCr() const;
	FRAMEWORK_DLL_API GLuint getTextureCb() const;
	FRAMEWORK_DLL_API int getWidth() const;
	FRAMEWORK_DLL_API int getHeight() const;
private:
	plm_t* plm;
	GLuint videoTextureY;
	GLuint videoTextureCr;
	GLuint videoTextureCb;
	int width;
	int height;
	double framerate;
	float frameDuration;
};
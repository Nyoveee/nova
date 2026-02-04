// Disable warnings for pl_mpeg (third-party header)
#pragma warning(push)
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'type', possible loss of data
#include <stdio.h>
#define PL_MPEG_IMPLEMENTATION
#include "Internal/pl_mpeg.h"
#pragma warning(pop)

#include "video.h"

#include <glad/glad.h>
#include <filesystem>
#include <limits>

#include "Logger.h"

namespace {
	GLuint TEXTURE_NOT_LOADED = std::numeric_limits<GLuint>::max();
}

Video::Video(ResourceID id, ResourceFilePath resourceFilePath) :
	Resource			{ id, std::move(resourceFilePath) },
	plm					{ nullptr },
	videoTextureY		{ TEXTURE_NOT_LOADED },
	videoTextureCr		{ TEXTURE_NOT_LOADED },
	videoTextureCb		{ TEXTURE_NOT_LOADED },
	width				{ 0 },
	height				{ 0 },
	timeAccumulator		{ 0.0f },
	looping				{ true }
{}

Video::~Video() {
	unload();
}

bool Video::load() {
	// Clean up existing video if any
	unload();

	// Initialize plmpeg with the video file
	std::string filepath = getFilePath().string;
	plm = plm_create_with_filename(filepath.c_str());
	if (!plm) {
		Logger::error("Failed to load video: {}", filepath);
		return false;
	}

	Logger::info("Video loaded successfully: {}", filepath);

	// Get video dimensions
	width = plm_get_width(plm);
	height = plm_get_height(plm);
	Logger::info("Video dimensions: {}x{}", width, height);

	// Generate textures
	glGenTextures(1, &videoTextureY);
	glGenTextures(1, &videoTextureCr);
	glGenTextures(1, &videoTextureCb);

	// Set up Y texture (full resolution)
	glBindTexture(GL_TEXTURE_2D, videoTextureY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set up Cr texture (half resolution)
	glBindTexture(GL_TEXTURE_2D, videoTextureCr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set up Cb texture (half resolution)
	glBindTexture(GL_TEXTURE_2D, videoTextureCb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Disable audio and set looping
	plm_set_audio_enabled(plm, false);
	plm_set_loop(plm, looping);

	timeAccumulator = 0.0f;

	return true;
}

void Video::unload() {
	if (plm) {
		plm_destroy(plm);
		plm = nullptr;
	}

	if (videoTextureY != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &videoTextureY);
		videoTextureY = TEXTURE_NOT_LOADED;
	}

	if (videoTextureCr != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &videoTextureCr);
		videoTextureCr = TEXTURE_NOT_LOADED;
	}

	if (videoTextureCb != TEXTURE_NOT_LOADED) {
		glDeleteTextures(1, &videoTextureCb);
		videoTextureCb = TEXTURE_NOT_LOADED;
	}

	width = 0;
	height = 0;
	timeAccumulator = 0.0f;
}

bool Video::decodeFrame(float dt) {
	if (!plm) return false;

	// Accumulate time
	timeAccumulator += dt;

	// Get video framerate and calculate frame duration
	double framerate = plm_get_framerate(plm);
	if (framerate <= 0.0) framerate = 30.0; // Default fallback
	float frameDuration = 1.0f / static_cast<float>(framerate);

	// Only decode if enough time has passed
	if (timeAccumulator < frameDuration) {
		return false;
	}

	// Decode the next frame
	plm_frame_t* frame = plm_decode_video(plm);
	if (!frame) {
		return false;
	}

	// Upload Y plane
	glBindTexture(GL_TEXTURE_2D, videoTextureY);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->y.width, frame->y.height, GL_RED, GL_UNSIGNED_BYTE, frame->y.data);

	// Upload Cr plane
	glBindTexture(GL_TEXTURE_2D, videoTextureCr);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->cr.width, frame->cr.height, GL_RED, GL_UNSIGNED_BYTE, frame->cr.data);

	// Upload Cb plane
	glBindTexture(GL_TEXTURE_2D, videoTextureCb);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->cb.width, frame->cb.height, GL_RED, GL_UNSIGNED_BYTE, frame->cb.data);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Subtract frame duration, keeping remainder for smooth playback
	timeAccumulator -= frameDuration;

	return true;
}

GLuint Video::getTextureY() const {
	return videoTextureY;
}

GLuint Video::getTextureCr() const {
	return videoTextureCr;
}

GLuint Video::getTextureCb() const {
	return videoTextureCb;
}

int Video::getWidth() const {
	return width;
}

int Video::getHeight() const {
	return height;
}

bool Video::isLoaded() const {
	return plm != nullptr;
}

void Video::setLoop(bool loop) {
	looping = loop;
	if (plm) {
		plm_set_loop(plm, looping);
	}
}

bool Video::getLoop() const {
	return looping;
}

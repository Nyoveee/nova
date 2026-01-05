#include <limits>
#include <glad/glad.h>

#include "depthFrameBuffer.h"
#include "Logger.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

DepthFrameBuffer::DepthFrameBuffer(int width, int height) :
	FBO_id			{ INVALID_ID },
	depthTextureId	{ INVALID_ID },
	width			{ width },
	height			{ height }
{
	glCreateFramebuffers(1, &FBO_id);

	// Create texture for depth component
	glCreateTextures(GL_TEXTURE_2D, 1, &depthTextureId);
	
	glTextureStorage2D(depthTextureId, 1, GL_DEPTH_COMPONENT32F, width, height);

	glTextureParameteri(depthTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthTextureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(depthTextureId, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Attach created texture as depth attachment for this framebuffer.
	glNamedFramebufferTexture(FBO_id, GL_DEPTH_ATTACHMENT, depthTextureId, 0);

	// We configure the FBO to not work with any draw / read buffer (color attachments).
	glNamedFramebufferDrawBuffer(FBO_id, GL_NONE);
	glNamedFramebufferReadBuffer(FBO_id, GL_NONE);

	if (glCheckNamedFramebufferStatus(FBO_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Logger::error("Error: Framebuffer incomplete!");
	}
}

DepthFrameBuffer::~DepthFrameBuffer() {
	if (FBO_id != INVALID_ID)			glDeleteFramebuffers(1, &FBO_id);
	if (depthTextureId != INVALID_ID)	glDeleteTextures(1, &depthTextureId);
}

DepthFrameBuffer::DepthFrameBuffer(DepthFrameBuffer&& other) noexcept :
	FBO_id			{ other.FBO_id },
	depthTextureId	{ other.depthTextureId },
	width			{ other.width },
	height			{ other.height }
{
	other.FBO_id = INVALID_ID;
	other.depthTextureId = INVALID_ID;
}

DepthFrameBuffer& DepthFrameBuffer::operator=(DepthFrameBuffer&& other) noexcept {
	DepthFrameBuffer tmp{ std::move(other) };
	swap(tmp);
	return *this;
}

void DepthFrameBuffer::swap(DepthFrameBuffer& rhs) {
	std::swap(FBO_id,			rhs.FBO_id);
	std::swap(depthTextureId,	rhs.depthTextureId);
	std::swap(width,			rhs.width);
	std::swap(height,			rhs.height);
}

GLuint DepthFrameBuffer::fboId() const {
	return FBO_id;
}

GLuint DepthFrameBuffer::textureId() const {
	return depthTextureId;
}
#include <glad/glad.h>
#include <algorithm>
#include <iostream>

#include "MSAAFramebuffer.h"
#include "Logger.h"
#include "framebuffer.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
	constexpr int NUM_OF_SAMPLES = 4;
}

MSAAFrameBuffer::MSAAFrameBuffer(int width, int height, TextureInternalFormat colorAttachmentProperty) :
	FBO_id						{ INVALID_ID },
	multiSampleColorTextureId	{ INVALID_ID },
	multiSampleDepthStencilId	{ INVALID_ID },
	width						{ width },
	height						{ height }
{
	glCreateFramebuffers(1, &FBO_id);

	// Creating my MSAA textures..
	// 1. color attachment
	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &multiSampleColorTextureId);
	glTextureStorage2DMultisample(multiSampleColorTextureId, NUM_OF_SAMPLES, colorAttachmentProperty, width, height, GL_TRUE);
	glNamedFramebufferTexture(FBO_id, GL_COLOR_ATTACHMENT0, multiSampleColorTextureId, 0);

	// 2. depth & stencil..
	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &multiSampleDepthStencilId);
	glTextureStorage2DMultisample(multiSampleDepthStencilId, NUM_OF_SAMPLES, GL_DEPTH24_STENCIL8, width, height, GL_TRUE);
	glNamedFramebufferTexture(FBO_id, GL_DEPTH_STENCIL_ATTACHMENT, multiSampleDepthStencilId, 0);

	GLenum status = glCheckNamedFramebufferStatus(FBO_id, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		Logger::error("[Ctor] MSAA FBO incomplete! Status: {}", status);
}

MSAAFrameBuffer::~MSAAFrameBuffer() {
	if (FBO_id != INVALID_ID)						glDeleteFramebuffers(1, &FBO_id);
	if (multiSampleColorTextureId != INVALID_ID)	glDeleteTextures(1, &multiSampleColorTextureId);
	if (multiSampleDepthStencilId != INVALID_ID)	glDeleteTextures(1, &multiSampleDepthStencilId);
}

MSAAFrameBuffer::MSAAFrameBuffer(MSAAFrameBuffer&& other) noexcept :
	FBO_id						{ other.FBO_id },
	width						{ other.width },
	height						{ other.height },
	multiSampleColorTextureId	{ other.multiSampleColorTextureId },
	multiSampleDepthStencilId	{ other.multiSampleDepthStencilId }
{
	other.FBO_id					= INVALID_ID;
	other.multiSampleColorTextureId = INVALID_ID;
	other.multiSampleDepthStencilId = INVALID_ID;
}

MSAAFrameBuffer& MSAAFrameBuffer::operator=(MSAAFrameBuffer&& other) noexcept {
	MSAAFrameBuffer tmp{ std::move(other) };
	swap(tmp);
	return *this;
}

void MSAAFrameBuffer::swap(MSAAFrameBuffer& rhs) {
	std::swap(FBO_id,						rhs.FBO_id);
	std::swap(width,						rhs.width);
	std::swap(height,						rhs.height);
	std::swap(multiSampleColorTextureId,	rhs.multiSampleColorTextureId);
	std::swap(multiSampleDepthStencilId,	rhs.multiSampleDepthStencilId);
}

GLuint MSAAFrameBuffer::fboId() const {
	return FBO_id;
}

GLuint MSAAFrameBuffer::depthStencilId() const {
	return multiSampleDepthStencilId;
}

int MSAAFrameBuffer::getWidth() const {
	return width;
}

int MSAAFrameBuffer::getHeight() const {
	return height;
}

void MSAAFrameBuffer::clear() {
	constexpr float defaultColor[4] = { 0.1f, 0.1f, 0.1f, 1.f };

	glClearNamedFramebufferfv(
		FBO_id,
		GL_COLOR,
		0,		// color attachment 0.
		defaultColor
	);

	glClearNamedFramebufferfi(
		FBO_id,
		GL_DEPTH_STENCIL,
		0,
		1.0f,   // depth
		0       // stencil
	);
}

void MSAAFrameBuffer::resolveColor(FrameBuffer& targetFrameBuffer) {
	glBlitNamedFramebuffer(FBO_id, targetFrameBuffer.fboId(), 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void MSAAFrameBuffer::resolveDepthStencil(FrameBuffer& targetFrameBuffer) {
	glBlitNamedFramebuffer(FBO_id, targetFrameBuffer.fboId(), 0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
}

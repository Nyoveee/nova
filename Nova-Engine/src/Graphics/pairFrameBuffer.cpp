#include <cassert>
#include <glad/glad.h>

#include "pairFrameBuffer.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

PairFrameBuffer::PairFrameBuffer(int width, int height, int mainColorAttachment, std::vector<TextureInternalFormat> additionalColorAttachments) :
	frameBuffers{ {{ width, height, mainColorAttachment }, { width, height, mainColorAttachment }}}
{
	// Create texture for each color attachments
	for (TextureInternalFormat textureFormat : additionalColorAttachments) {
		GLuint texture_id = INVALID_ID;

		glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);

		glTextureStorage2D(texture_id, 1, textureFormat, width, height);

		glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		texture_ids.push_back(texture_id);
	}
}

PairFrameBuffer::~PairFrameBuffer() {
	for (GLuint texture_id : texture_ids) {
		glDeleteTextures(1, &texture_id);
	}
}

void PairFrameBuffer::clearFrameBuffers() {
	constexpr float color[4] = { 0.f, 0.f, 0.f, 1.f };

	// Clear both main framebuffers.
	for (auto&& framebuffer : frameBuffers) {
		framebuffer.clear();
	}

	// We clear the color attachments..
	for (int i = 1; i < textureIds().size(); ++i) {
		GLuint textureId = textureIds()[i];
		glClearTexImage(textureId, 0, GL_RGBA, GL_FLOAT, color);
	}

	// We choose 1 as the active index because we last bind to the last element of the array above^
	// Reset main frame buffer indices..
	activeFrameBufferIndex = 1;
	readFrameBufferIndex = 0;

	attachColorAttachments();
	attachDepthAttachment();
}

void PairFrameBuffer::swapFrameBuffer() {
	if (activeFrameBufferIndex == 0) {
		activeFrameBufferIndex = 1;
		readFrameBufferIndex = 0;
	}
	else if (activeFrameBufferIndex == 1) {
		activeFrameBufferIndex = 0;
		readFrameBufferIndex = 1;
	}
	else {
		assert(false && "Invalid index.");
	}
}

void PairFrameBuffer::attachColorAttachments() {
	if (currentAttachmentIndex != activeFrameBufferIndex) {
		for (int i = 0; i < texture_ids.size(); ++i) {
			GLuint colorAttachment = GL_COLOR_ATTACHMENT0 + i + 1;
			glNamedFramebufferTexture(getActiveFrameBuffer().fboId(), colorAttachment, texture_ids[i], 0);
		}

		currentAttachmentIndex = activeFrameBufferIndex;
	}
}

void PairFrameBuffer::attachDepthAttachment() {
	if (currentDepthIndex != activeFrameBufferIndex) {
		glNamedFramebufferTexture(getActiveFrameBuffer().fboId(), GL_DEPTH_STENCIL_ATTACHMENT, frameBuffers[1].depthStencilId(), 0);
		currentDepthIndex = activeFrameBufferIndex;
	}
}

FrameBuffer const& PairFrameBuffer::getActiveFrameBuffer() const {
	return frameBuffers[activeFrameBufferIndex];
}

FrameBuffer const& PairFrameBuffer::getReadFrameBuffer() const {
	return frameBuffers[readFrameBufferIndex];
}

GLuint PairFrameBuffer::getDepthTextureId() const {
	return frameBuffers[1].depthStencilId();
}

std::vector<GLuint> const& PairFrameBuffer::textureIds() const {
	return texture_ids;
}

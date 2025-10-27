#include <cassert>
#include <glad/glad.h>

#include "pairFrameBuffer.h"

PairFrameBuffer::PairFrameBuffer(int width, int height, std::vector<TextureInternalFormat> colorAttachments) :
	frameBuffers{ {{ width, height, colorAttachments }, { width, height, colorAttachments }} }
{}

void PairFrameBuffer::clearFrameBuffers() {
	// Clear both main framebuffers.
	for (auto&& framebuffer : frameBuffers) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fboId());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	// We choose 1 as the active index because we last bind to the last element of the array above^
	// Reset main frame buffer indices..
	activeFrameBufferIndex = 1;
	readFrameBufferIndex = 0;
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

FrameBuffer const& PairFrameBuffer::getActiveFrameBuffer() const {
	return frameBuffers[activeFrameBufferIndex];
}

FrameBuffer const& PairFrameBuffer::getReadFrameBuffer() const {
	return frameBuffers[readFrameBufferIndex];
}

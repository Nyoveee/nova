#pragma once

#include "framebuffer.h"
#include <array>

/*
	The idea of a PairFrameBuffer is that we store 2 framebuffers internal, and ping pong between one another
	when doing post processing passes.

	We are able to retrieve the active framebuffer, for us to write and contains the final result.
	We can also retrieve the read framebuffer, for us to read from and write to the active framebuffer during a post processing pass.

	In any typical post processing pass, the swapFrameBuffer() function is first invoked, changing the active framebuffer to the read framebuffer.
	The new active framebuffer is now available for writing, and the read framebuffer can be read from.

	P.S i like writing documentation.
*/
class PairFrameBuffer {
public:
	PairFrameBuffer(int width, int height, std::vector<TextureInternalFormat> colorAttachments = { GL_RGBA16 });

public:
	void clearFrameBuffers();
	void swapFrameBuffer();

	FrameBuffer const& getActiveFrameBuffer() const;
	FrameBuffer const& getReadFrameBuffer() const;

private:
	std::array<FrameBuffer, 2> frameBuffers;
	int activeFrameBufferIndex	= 1;
	int readFrameBufferIndex	= 0;
};
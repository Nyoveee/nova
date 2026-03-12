#pragma once

#include "framebuffer.h"
#include <array>

#include "export.h"
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
	PairFrameBuffer(int width, int height, int mainColorAttachments, std::vector<TextureInternalFormat> additionalColorAttachments = {});
	~PairFrameBuffer();

	PairFrameBuffer(PairFrameBuffer const& other)				= delete;
	PairFrameBuffer(PairFrameBuffer&& other)					= delete;
	PairFrameBuffer& operator=(PairFrameBuffer const& other)	= delete;
	PairFrameBuffer& operator=(PairFrameBuffer&& other)			= delete;

public:
	void clearFrameBuffers();

	void swapFrameBuffer();

	// swapping frame buffers means that the color attachments are no longer attached to the active FBO.
	// invoke this member function to attach the additional color attachments to the active FBO.
	void attachColorAttachments();
	void attachDepthAttachment();

	FrameBuffer const& getActiveFrameBuffer() const;
	FrameBuffer const& getReadFrameBuffer() const;

	GLuint getDepthTextureId() const;

	std::vector<GLuint> const& textureIds()	 const;

private:
	std::array<FrameBuffer, 2> frameBuffers;
	int activeFrameBufferIndex	= 1;
	int readFrameBufferIndex	= 0;

	std::vector<GLuint> texture_ids;
	int currentAttachmentIndex = -1;
	int currentDepthIndex = 1;
};
#pragma once

#include "framebuffer.h"

class SSAOFrameBuffer : public FrameBuffer
{
public:
	SSAOFrameBuffer(int width, int height);

	~SSAOFrameBuffer();
	SSAOFrameBuffer(SSAOFrameBuffer const& other) = delete;
	SSAOFrameBuffer(SSAOFrameBuffer&& other) = delete;
	SSAOFrameBuffer& operator=(SSAOFrameBuffer const& other) = delete;
	SSAOFrameBuffer& operator=(SSAOFrameBuffer&& other) = delete;

public:

private:
	GLuint depthTextureId;
	GLuint normalTextureId;
	GLuint positionTextureId;
};
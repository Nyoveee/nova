#pragma once

#include <glm/vec2.hpp>
#include <vector>

#include "framebuffer.h"

class BloomFrameBuffer : public FrameBuffer
{
public:
	struct MipMapTexture {
		glm::vec2 size;
		glm::ivec2 isize;
		GLuint id;
	};

public:
	BloomFrameBuffer(int width, int height, int mipmapChain);
	
	~BloomFrameBuffer();
	BloomFrameBuffer(BloomFrameBuffer const& other)				= delete;
	BloomFrameBuffer(BloomFrameBuffer&& other)					= delete;
	BloomFrameBuffer& operator=(BloomFrameBuffer const& other)	= delete;
	BloomFrameBuffer& operator=(BloomFrameBuffer&& other)		= delete;

public:
	std::vector<MipMapTexture> const& getMipChain() const;

private:
	glm::ivec2 gameSize;
	std::vector<MipMapTexture> mipChain;
};
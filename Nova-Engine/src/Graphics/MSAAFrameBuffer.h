#pragma once

#include <limits>
#include <vector>

class FrameBuffer;

using GLuint = unsigned int;
using TextureInternalFormat = int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class MSAAFrameBuffer {
public:
	MSAAFrameBuffer(int width, int height, TextureInternalFormat colorAttachmentProperty);

	~MSAAFrameBuffer();
	MSAAFrameBuffer(MSAAFrameBuffer const& other) = delete;
	MSAAFrameBuffer(MSAAFrameBuffer&& other) noexcept;
	MSAAFrameBuffer& operator=(MSAAFrameBuffer const& other) = delete;
	MSAAFrameBuffer& operator=(MSAAFrameBuffer&& other) noexcept;

	void swap(MSAAFrameBuffer& rhs);
	
public:
	void clear();

	// performs MSAA resolve to the target FBO..
	void resolveColor(FrameBuffer& targetFrameBuffer);
	void resolveDepthStencil(FrameBuffer& targetFrameBuffer);

public:
	GLuint						fboId()			 const;
	GLuint						depthStencilId() const;
	int							getWidth()		 const;
	int							getHeight()		 const;

private:
	GLuint FBO_id;

	GLuint multiSampleColorTextureId;
	GLuint multiSampleDepthStencilId;

	int width;
	int height;
};
#pragma once

#include <limits>
#include <vector>

using GLuint = unsigned int;
using TextureInternalFormat = int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class FrameBuffer {
public:
	// Constructs a FBO with dimension of width and height, along with N amount of color attachments.
	// where N = size of vector `colorAttachments`
	// Multiple color attachments are used for MRT.
	// each element specifies the internal format of the color attachment. 
	FrameBuffer(int width, int height, int internalFormat);

	~FrameBuffer();
	FrameBuffer(FrameBuffer const& other) = delete;
	FrameBuffer(FrameBuffer&& other) noexcept;
	FrameBuffer& operator=(FrameBuffer const& other) = delete;
	FrameBuffer& operator=(FrameBuffer&& other) noexcept;

	void swap(FrameBuffer& rhs);

	// disable all other color attachments, only allowing rendering to the main color attachment.
	void disableOtherColorAttachments() const;

	void clear();

public:
	GLuint	fboId()			 const;
	GLuint	textureId()		 const;
	GLuint	depthStencilId() const;
	int		getWidth()		 const;
	int		getHeight()		 const;

private:
	GLuint FBO_id;
	GLuint texture_id; // holds the texture of the respective color attachments
	GLuint depthStencilTextureId;

	int width;
	int height;
};
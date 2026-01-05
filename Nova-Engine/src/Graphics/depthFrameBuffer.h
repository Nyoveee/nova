#pragma once

using GLuint = unsigned int;

class DepthFrameBuffer {
public:
	// Constructs a FBO with dimension of width and height, along with N amount of color attachments.
	// where N = size of vector `colorAttachments`
	// Multiple color attachments are used for MRT.
	// each element specifies the internal format of the color attachment. 
	DepthFrameBuffer(int width, int height);

	~DepthFrameBuffer();
	DepthFrameBuffer(DepthFrameBuffer const& other) = delete;
	DepthFrameBuffer(DepthFrameBuffer&& other) noexcept;
	DepthFrameBuffer& operator=(DepthFrameBuffer const& other) = delete;
	DepthFrameBuffer& operator=(DepthFrameBuffer&& other) noexcept;

	void swap(DepthFrameBuffer& rhs);

public:
	GLuint fboId()		const;
	GLuint textureId()	const;

private:
	GLuint FBO_id;
	GLuint depthTextureId; // holds the texture of the depth map.

	int width;
	int height;
};
#pragma once

#include <limits>

using GLuint = unsigned int;

// We use the copy-and-swap idiom (but really move and swap idiom since copy semantics are disabled) to implement move semantics.
class FrameBuffer {
public:
	FrameBuffer(int width, int height);

	~FrameBuffer();
	FrameBuffer(FrameBuffer const& other) = delete;
	FrameBuffer(FrameBuffer&& other) noexcept;
	FrameBuffer& operator=(FrameBuffer const& other) = delete;
	FrameBuffer& operator=(FrameBuffer&& other) noexcept;

	void swap(FrameBuffer& rhs);

public:
	GLuint fboId() const;
	GLuint textureId() const;
	GLuint rboId() const;

private:
	GLuint FBO_id;
	GLuint texture_id;
	GLuint RBO_id;

};
#include <glad/glad.h>
#include <algorithm>
#include <iostream>

#include "framebuffer.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

FrameBuffer::FrameBuffer(int width, int height) : 
	FBO_id		{ INVALID_ID },
	texture_id	{ INVALID_ID },
	RBO_id		{ INVALID_ID }
{
	glCreateFramebuffers(1, &FBO_id);

	// Generating FBO texture attachment
	glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);

	glTextureStorage2D(texture_id, 1, GL_RGBA16F, width, height);
	//glTextureSubImage2D(texture_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glNamedFramebufferTexture(FBO_id, GL_COLOR_ATTACHMENT0, texture_id, 0);

	// Generating renderbuffer object for depth / stencil testing
	glCreateRenderbuffers(1, &RBO_id);
	glNamedRenderbufferStorage(RBO_id, GL_DEPTH24_STENCIL8, width, height);

	// Attaching renderbuffer object to depth and stencil attachment of framebuffer
	glNamedFramebufferRenderbuffer(FBO_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_id);

	if (glCheckNamedFramebufferStatus(FBO_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: Framebuffer incomplete!\n";
	}
}

FrameBuffer::~FrameBuffer() {
	if(FBO_id != INVALID_ID)		glDeleteFramebuffers(1, &FBO_id);
	if(texture_id != INVALID_ID)	glDeleteTextures(1, &texture_id);
	if(RBO_id != INVALID_ID)		glDeleteRenderbuffers(1, &RBO_id);
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept :
	FBO_id		{ other.FBO_id },
	texture_id	{ other.texture_id },
	RBO_id		{ other.RBO_id }
{
	other.FBO_id		= INVALID_ID;
	other.texture_id	= INVALID_ID;
	other.RBO_id		= INVALID_ID;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	FrameBuffer tmp{ std::move(other) };
	swap(tmp);
	return *this;
}

void FrameBuffer::swap(FrameBuffer& rhs) {
	std::swap(FBO_id,		rhs.FBO_id);
	std::swap(texture_id,	rhs.texture_id);
	std::swap(RBO_id,		rhs.RBO_id);
}

GLuint FrameBuffer::fboId() const {
	return FBO_id;
}

GLuint FrameBuffer::textureId() const {
	return texture_id;
}

GLuint FrameBuffer::rboId() const {
	return RBO_id;
}

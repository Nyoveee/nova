#include <glad/glad.h>
#include <algorithm>
#include <iostream>

#include "framebuffer.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

// =============================================
// A frame buffer consist of 
// - multiple color attachments (we use textures for each color attachment)
// - depth buffer		--|
// - stencil buffer		--|-> both are implemented using a renderbuffer object, 3 bytes for depth and 1 byte for stencil
// 
// Each color attachments can have their own internal format. For an example,
// COLOR_ATTACHMENT0 can be used for colors, having an internal format of GL_RGBA16
// COLOR_ATTACHMENT1 can be used for object id, having an internal format of GL_R32UI
// 
// We specify how many color attachments and the properties of the color attachment through the parameter 
// `colorAttachmentProperties`
// 
// For an example, 
// - FrameBuffer{800, 400, { GL_RGBA16, GLR32UI });
// Creates a frame buffer with 2 color attachments, with COLOR_ATTACHMENT0 as GL_RGBA16 and COLOR_ATTACHMENT1 as GLR32UI.
// =============================================
FrameBuffer::FrameBuffer(int width, int height, std::vector<int> colorAttachmentProperties) :
	FBO_id		{ INVALID_ID },
	texture_ids	{},
	RBO_id		{ INVALID_ID },
	width		{ width },
	height		{ height }
{
	if (colorAttachmentProperties.size() > 8) {
		std::cerr << "Too many render targets specified.\n";
		return;
	}

	texture_ids.resize(colorAttachmentProperties.size(), INVALID_ID);
	glCreateFramebuffers(1, &FBO_id);

	unsigned int i = 0;
	std::vector<GLuint> colorAttachments;

	// Create texture for each color attachments
	for (TextureInternalFormat textureFormat : colorAttachmentProperties) {
		GLuint& texture_id = texture_ids[i];
		glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);

		glTextureStorage2D(texture_id, 1, textureFormat, width, height);

		glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		GLuint colorAttachment = GL_COLOR_ATTACHMENT0 + i;
		colorAttachments.push_back(colorAttachment);
		glNamedFramebufferTexture(FBO_id, colorAttachment, texture_id, 0);
		++i;
	}

	// link respective color attachments to draw buffers in a multi render target framebuffer..
	if (colorAttachments.size() > 1) {
		glNamedFramebufferDrawBuffers(FBO_id, static_cast<GLsizei>(colorAttachments.size()), colorAttachments.data());
	}

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
	if(RBO_id != INVALID_ID)		glDeleteRenderbuffers(1, &RBO_id);

	for (GLuint texture_id : texture_ids) {
		glDeleteTextures(1, &texture_id);
	}
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept :
	FBO_id		{ other.FBO_id },
	texture_ids	{ std::move(other.texture_ids) },
	RBO_id		{ other.RBO_id }
{
	other.FBO_id		= INVALID_ID;
	other.RBO_id		= INVALID_ID;

	other.texture_ids.clear();
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	FrameBuffer tmp{ std::move(other) };
	swap(tmp);
	return *this;
}

void FrameBuffer::swap(FrameBuffer& rhs) {
	std::swap(FBO_id,		rhs.FBO_id);
	std::swap(texture_ids,	rhs.texture_ids);
	std::swap(RBO_id,		rhs.RBO_id);
}

GLuint FrameBuffer::fboId() const {
	return FBO_id;
}

std::vector<GLuint> const& FrameBuffer::textureIds() const {
	return texture_ids;
}

GLuint FrameBuffer::rboId() const {
	return RBO_id;
}

int FrameBuffer::getWidth() const {
	return width;
}

int FrameBuffer::getHeight() const {
	return height;
}

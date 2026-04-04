#include <glad/glad.h>
#include <algorithm>
#include <iostream>

#include "framebuffer.h"
#include "Logger.h"
#include "depthFrameBuffer.h"

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

FrameBuffer::FrameBuffer(int width, int height, int internalFormat) :
	FBO_id					{ INVALID_ID },
	texture_id				{ INVALID_ID },
	depthStencilTextureId	{ INVALID_ID },
	width					{ width },
	height					{ height }
{
	glCreateFramebuffers(1, &FBO_id);

	// Create texture for each color attachments
	glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);

	glTextureStorage2D(texture_id, 1, internalFormat, width, height);

	glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint colorAttachment = GL_COLOR_ATTACHMENT0;
	glNamedFramebufferTexture(FBO_id, colorAttachment, texture_id, 0);

	// Generating a texture object for depth / stencil testing
	glCreateTextures(GL_TEXTURE_2D, 1, &depthStencilTextureId);
	glTextureStorage2D(depthStencilTextureId, 1, GL_DEPTH24_STENCIL8, width, height);

	// Attaching renderbuffer object to depth and stencil attachment of framebuffer
	glNamedFramebufferTexture(FBO_id, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilTextureId, 0);

	if (glCheckNamedFramebufferStatus(FBO_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Logger::error("Error: Framebuffer incomplete!");
	}
}

FrameBuffer::~FrameBuffer() {
	if(FBO_id != INVALID_ID)					glDeleteFramebuffers(1, &FBO_id);
	if(depthStencilTextureId != INVALID_ID)		glDeleteTextures(1, &depthStencilTextureId);
	if (texture_id != INVALID_ID)				glDeleteTextures(1, &texture_id);
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept :
	FBO_id					{ other.FBO_id },
	texture_id				{ other.texture_id },
	depthStencilTextureId	{ other.depthStencilTextureId },
	width					{ other.width },
	height					{ other.height }
{
	other.FBO_id				= INVALID_ID;
	other.depthStencilTextureId = INVALID_ID;
	other.texture_id			= INVALID_ID;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
	FrameBuffer tmp{ std::move(other) };
	swap(tmp);
	return *this;
}

void FrameBuffer::swap(FrameBuffer& rhs) {
	std::swap(FBO_id,					rhs.FBO_id);
	std::swap(texture_id,				rhs.texture_id);
	std::swap(depthStencilTextureId,	rhs.depthStencilTextureId);
	std::swap(width,					rhs.width);
	std::swap(height,					rhs.height);
}

#if 0
void FrameBuffer::setColorAttachmentActive(int number) const {
	// link respective color attachments to draw buffers in a multi render target framebuffer..
	glNamedFramebufferDrawBuffers(FBO_id, static_cast<GLsizei>(number), colorAttachments.data());
}
#endif

GLuint FrameBuffer::fboId() const {
	return FBO_id;
}

GLuint FrameBuffer::textureId() const {
	return texture_id;
}

void FrameBuffer::disableOtherColorAttachments() const {
	static constexpr GLuint colorAttachments[] = { GL_COLOR_ATTACHMENT0 };
	glNamedFramebufferDrawBuffers(FBO_id, 1, colorAttachments);
}

GLuint FrameBuffer::depthStencilId() const {
	return depthStencilTextureId;
}

int FrameBuffer::getWidth() const {
	return width;
}

int FrameBuffer::getHeight() const {
	return height;
}

void FrameBuffer::clear() {
	// https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
	constexpr float defaultColor[4] = { 0.00f, 0.00f, 0.00f, 1.f };
	//constexpr float defaultColor[4] = { 0.05f, 0.05f, 0.05f, 1.f };

	glBindFramebuffer(GL_FRAMEBUFFER, FBO_id);
	
	glClearTexImage(texture_id, 0, GL_RGBA, GL_FLOAT, defaultColor);

	// Clear depth to 1.0, stencil to 0
	glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0); 
}
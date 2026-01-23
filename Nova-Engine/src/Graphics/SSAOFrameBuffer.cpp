#include <glad/glad.h>
#include "SSAOFrameBuffer.h"

namespace {
	// A FBO holding INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

SSAOFrameBuffer::SSAOFrameBuffer(int width, int height) :
	FrameBuffer			{ width, height, {} },
	depthTextureId		{ INVALID_ID },
	normalTextureId		{ INVALID_ID },
	positionTextureId	{ INVALID_ID }
{
	// Create our depth texture..
	glCreateTextures(GL_TEXTURE_2D, 1, &depthTextureId);

	glTextureStorage2D(depthTextureId, 1, GL_DEPTH_COMPONENT24, width, height);
	glTextureParameteri(depthTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(depthTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(depthTextureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthTextureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create our normal texture..
	glCreateTextures(GL_TEXTURE_2D, 1, &normalTextureId);

	glTextureStorage2D(normalTextureId, 1, GL_RGB8, width, height);
	glTextureParameteri(normalTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(normalTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(normalTextureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(normalTextureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create our position texture..
	glCreateTextures(GL_TEXTURE_2D, 1, &positionTextureId);

	glTextureStorage2D(positionTextureId, 1, GL_RGB32F, width, height);
	glTextureParameteri(positionTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(positionTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(positionTextureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(positionTextureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

SSAOFrameBuffer::~SSAOFrameBuffer() {
	if (depthTextureId)		glDeleteTextures(1, &depthTextureId);
	if (normalTextureId)	glDeleteTextures(1, &normalTextureId);
	if (positionTextureId)	glDeleteTextures(1, &positionTextureId);
}

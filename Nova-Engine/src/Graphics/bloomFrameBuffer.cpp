#include <glad/glad.h>
#include "bloomFrameBuffer.h"

BloomFrameBuffer::BloomFrameBuffer(int width, int height, int mipmapChain) :
	FrameBuffer { width, height, {} },
	gameSize	{ width, height }
{
	// Create my "mipmap" chain.
	glm::vec2	mipSize		{ (float)width, (float)height };
	glm::ivec2	mipIntSize	{ width, height };

	mipChain.reserve(mipmapChain);

	for (int i = 0; i < mipmapChain; i++) {
		MipMapTexture texture;

		// Half the size..
		mipSize *= 0.5f;
		mipIntSize /= 2;
		texture.size = mipSize;
		texture.isize = mipIntSize;

		// Create our mipmap texture..
		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

		// we are downscaling an HDR color buffer, so we need a float texture format
		glTextureStorage2D(texture.id, 1, GL_R11F_G11F_B10F, mipIntSize.x, mipIntSize.y);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		mipChain.push_back(texture);
	}
}

BloomFrameBuffer::~BloomFrameBuffer() {
	for (auto&& texture : mipChain) {
		glDeleteTextures(1, &texture.id);
	}
}

std::vector<BloomFrameBuffer::MipMapTexture> const& BloomFrameBuffer::getMipChain() const {
	return mipChain;
}

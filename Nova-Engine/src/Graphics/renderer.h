#pragma once

#include <glad/glad.h>
#include <vector>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "vertexBufferObject.h"
#include "framebuffer.h"

class Engine;

class Renderer {
public:
	enum class BlendingConfig {
		AlphaBlending,
		AdditiveBlending,
		PureAdditiveBlending,
		PremultipliedAlpha,
		Disabled
	};

	enum class RenderTarget {
		ToDefaultFrameBuffer,
		ToMainFrameBuffer
	};

public:
	Renderer(Engine& engine, int gameWidth, int gameHeight);

	~Renderer();
	Renderer(Renderer const& other)				= delete;
	Renderer(Renderer&& other)					= delete;
	Renderer& operator=(Renderer const& other)	= delete;
	Renderer& operator=(Renderer&& other)		= delete;

public:
	// used directly in the editor. i need to export this.
	DLL_API GLuint getMainFrameBufferTexture() const;

public:
	void update(float dt);

	// choose to either render to the default frame buffer or the main frame buffer.
	void render(RenderTarget target);

	Camera& getCamera();
	Camera const& getCamera() const;

private:
	void setBlendMode(BlendingConfig configuration);

private:
	Engine& engine;

	std::vector<VertexBufferObject> VBOs;

	GLuint VAO;
	GLuint EBO;

	Camera camera;

	Shader basicShader;
	Shader standardShader;
	Shader textureShader;

	FrameBuffer mainFrameBuffer;
};
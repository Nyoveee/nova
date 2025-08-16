#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "bufferObject.h"
#include "framebuffer.h"

class Engine;
class AssetManager;

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
	
	DLL_API std::vector<GLuint> const& getMainFrameBufferTextures() const;
	DLL_API void enableWireframeMode(bool toEnable) const;

	// gets object id from color attachment 1 of the main framebuffer.
	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

public:
	void update(float dt);

	// choose to either render to the default frame buffer or the main frame buffer.
	void render(RenderTarget target);

	Camera& getCamera();
	Camera const& getCamera() const;

private:
	void setBlendMode(BlendingConfig configuration);
	void printOpenGLDriverDetails() const;

private:
	Engine& engine;
	AssetManager& assetManager;

	std::vector<BufferObject> VBOs;
	
	BufferObject EBO;
	GLuint VAO;

	Camera camera;

	// contains all the final rendering.
	FrameBuffer mainFrameBuffer;

public:
	Shader basicShader;
	Shader standardShader;
	Shader textureShader;
	Shader gridShader;
	Shader outlineShader;
};
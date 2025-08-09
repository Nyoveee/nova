#pragma once

#include <glad/glad.h>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "vertexBufferObject.h"

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

public:
	Renderer(Engine& engine);

	~Renderer();
	Renderer(Renderer const& other)				= delete;
	Renderer(Renderer&& other)					= delete;
	Renderer& operator=(Renderer const& other)	= delete;
	Renderer& operator=(Renderer&& other)		= delete;

public:
	void update(float dt);
	void render();

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
};
#pragma once

#include <glad/glad.h>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "vertexBufferObject.h"

class Renderer {
	Renderer();

public:
	static Renderer& instance();

	~Renderer();
	Renderer(Renderer const& other)				= delete;
	Renderer(Renderer&& other)					= delete;
	Renderer& operator=(Renderer const& other)	= delete;
	Renderer& operator=(Renderer&& other)		= delete;

public:
	void update(float dt);
	void render() const;

	Camera& getCamera();
	Camera const& getCamera() const;

private:
	std::vector<VertexBufferObject> VBOs;

	GLuint VAO;
	GLuint EBO;

	Camera camera;

	Shader basicShader;
	Shader standardShader;
};
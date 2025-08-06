#pragma once

#include <glad/glad.h>
#include "shader.h"

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
	void render() const;

private:
	GLuint VBO;
	GLuint VAO;
	GLuint EBO;

	Shader basicShader;
};
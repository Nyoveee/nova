#include <glm/vec2.hpp>

#include <array>

#include "Renderer.h"

Renderer& Renderer::instance() {
	static Renderer renderer{};
	return renderer;
}

Renderer::Renderer() : 
	basicShader { "Assets/Shader/basic.vert", "Assets/Shader/basic.frag" },
	VAO			{},
	VBO			{},
	EBO			{}
{
	struct Vertex {
		glm::vec2 pos;
		glm::vec2 textureUnit;
	};

	std::array<Vertex, 6> vertices{
		Vertex{{  0.5f,  0.5f }, { 1.0f, 1.0f }},	// top right
		Vertex{{ -0.5f, -0.5f }, { 0.0f, 0.0f }},	// bottom left
		Vertex{{  0.5f, -0.5f }, { 1.0f, 0.0f }},	// bottom right
		
		Vertex{{ -0.5f,  0.5f }, { 0.0f, 1.0f }},	// top left
		Vertex{{ -0.5f, -0.5f }, { 0.0f, 0.0f }},	// bottom left
		Vertex{{  0.5f,  0.5f }, { 1.0f, 1.0f }},	// top right
	};

	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);
	glCreateVertexArrays(1, &VAO);

	// copy data to VBO.
	glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// for this VAO, associate bindingIndex 0 with this VBO. 
	GLuint bindingIndex = 0;
	glVertexArrayVertexBuffer(VAO, bindingIndex, VBO, 0, sizeof(Vertex));

	// associate attribute index 0 and 1 with the respective attribute properties.
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureUnit));

	// enable attributes
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);

	// associate attribute 0 and 1 to binding index 0. 
	glVertexArrayAttribBinding(VAO, 0, bindingIndex);
	glVertexArrayAttribBinding(VAO, 1, bindingIndex);
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Renderer::render() const {
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	basicShader.use();
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

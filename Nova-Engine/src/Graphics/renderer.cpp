#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include "renderer.h"
#include "../Export/Header/window.h"
#include "vertex.h"
#include "../ECS & Components/ECS.h"
#include "../ECS & Components/component.h"

Renderer& Renderer::instance() {
	static Renderer renderer{};
	return renderer;
}

Renderer::Renderer() : 
	basicShader			{ "Assets/Shader/basic.vert", "Assets/Shader/basic.frag" },
	standardShader		{ "Assets/Shader/standard.vert", "Assets/Shader/basic.frag" },
	VAO					{},
	EBO					{},
	camera				{}
{
	// construct a VBO. Allocate 120 bytes of memory to this VBO.
	VBOs.push_back({ 120 });

#if 0
	std::array<Vertex, 6> vertices{
		Vertex{{  0.5f,  0.5f,  0.f }, { 1.0f, 1.0f }},	// top right
		Vertex{{ -0.5f, -0.5f,  0.f }, { 0.0f, 0.0f }},	// bottom left
		Vertex{{  0.5f, -0.5f,  0.f }, { 1.0f, 0.0f }},	// bottom right
							    
		Vertex{{ -0.5f,  0.5f,  0.f }, { 0.0f, 1.0f }},	// top left
		Vertex{{ -0.5f, -0.5f,  0.f }, { 0.0f, 0.0f }},	// bottom left
		Vertex{{  0.5f,  0.5f,  0.f }, { 1.0f, 1.0f }},	// top right
	};

	glNamedBufferData(VBOs[0].id(), vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
#endif

	glCreateBuffers(1, &EBO);
	glCreateVertexArrays(1, &VAO);

	// for this VAO, associate bindingIndex 0 with this VBO. 
	GLuint bindingIndex = 0;
	glVertexArrayVertexBuffer(VAO, bindingIndex, VBOs[0].id(), 0, sizeof(Vertex));

	// associate attribute index 0 and 1 with the respective attribute properties.
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureUnit));

	// enable attributes
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);

	// associate attribute 0 and 1 to binding index 0. 
	glVertexArrayAttribBinding(VAO, 0, bindingIndex);
	glVertexArrayAttribBinding(VAO, 1, bindingIndex);

	// enable backface culling & depth testing.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
}

void Renderer::update(float dt) {
	(void) dt;
}

void Renderer::render() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	// for this VAO, associate bindingIndex 0 with 1st VBO.
	glVertexArrayVertexBuffer(VAO, 0, VBOs[0].id(), 0, sizeof(Vertex));

	// Model matrix.
	glm::vec3 pos = { 0.f, 0.f, -3.f};
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	glm::mat4x4 modelMatrix = glm::mat4x4{ 1 };
	modelMatrix = glm::translate(modelMatrix, pos);
	modelMatrix = glm::scale(modelMatrix, scale);

	// View matrix.
	glm::mat4x4 viewMatrix = camera.view();

	// Perspective matrix.
	glm::mat4x4 perspectiveMatrix = camera.projection();

	standardShader.use();
	standardShader.setMatrix("model", modelMatrix);
	standardShader.setMatrix("view", viewMatrix);
	standardShader.setMatrix("projection", perspectiveMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
#endif

	standardShader.use();
	standardShader.setMatrix("view", camera.view());
	standardShader.setMatrix("projection", camera.projection());

	// Retrieve all game objects and prepare them for batch rendering..
	entt::registry& registry = ECS::instance().registry;

	for (auto&& [entity, transform, mesh] : registry.view<Transform, Mesh>().each()) {
		// Model matrix.
		glm::mat4x4 modelMatrix = glm::mat4x4{ 1 };
		modelMatrix = glm::translate(modelMatrix, transform.position);
		modelMatrix = glm::scale(modelMatrix, transform.scale);

		// Copy mesh to VBO..
		//glNamedBufferSubData(VBOs[1].id(), 0, 6 * sizeof(decltype(mesh.vertices)::value_type), mesh.vertices.data());
		auto& VBO = VBOs[0];
		VBO.uploadData(mesh.vertices);

		standardShader.setMatrix("model", modelMatrix);

		// set VBO to VAO's [0] binding index.
		glVertexArrayVertexBuffer(VAO, 0, VBOs[0].id(), 0, sizeof(Vertex));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

Camera& Renderer::getCamera() {
	return camera;
}

Camera const& Renderer::getCamera() const {
	return camera;
}

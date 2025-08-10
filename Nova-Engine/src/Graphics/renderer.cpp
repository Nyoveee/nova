#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include "renderer.h"
#include "window.h"
#include "vertex.h"
#include "ECS.h"
#include "engine.h"
#include "assetManager.h"

#include "Component/component.h"

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine				{ engine },
	basicShader			{ "Assets/Shader/basic.vert", "Assets/Shader/basic.frag" },
	standardShader		{ "Assets/Shader/standard.vert", "Assets/Shader/basic.frag" },
	textureShader		{ "Assets/Shader/standard.vert", "Assets/Shader/image.frag" },
	VAO					{},
	EBO					{},
	camera				{},
	mainFrameBuffer		{ gameWidth, gameHeight }
{
	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);

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

	setBlendMode(BlendingConfig::AlphaBlending);
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
}

void Renderer::update(float dt) {
	(void) dt;
}

void Renderer::render(RenderTarget target) {
	// Clear default framebuffer regardless.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind to and clear main frame buffer if in used.
	switch (target)
	{
	case Renderer::RenderTarget::ToMainFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.fboId());
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;
	}
	
	textureShader.use();
	textureShader.setMatrix("view", camera.view());
	textureShader.setMatrix("projection", camera.projection());

	// Set texture..
	auto [asset, result] = engine.assetManager.getAsset<Texture>(0);

	if (!asset) {
		std::cerr << "Error retrieving asset!\n";
	}
	else {
		textureShader.setImageUniform("image", 0);
		glBindTextureUnit(0, asset->getTextureId());
	}
	
	// Retrieve all game objects and prepare them for batch rendering..
	entt::registry& registry = engine.ecs.registry;

	for (auto&& [entity, transform, mesh] : registry.view<Transform, Mesh>().each()) {
		// Model matrix.
		glm::mat4x4 modelMatrix = glm::mat4x4{ 1 };
		modelMatrix = glm::translate(modelMatrix, transform.position);
		modelMatrix = glm::scale(modelMatrix, transform.scale);

		// Copy mesh to VBO..
		//glNamedBufferSubData(VBOs[1].id(), 0, 6 * sizeof(decltype(mesh.vertices)::value_type), mesh.vertices.data());
		auto& VBO = VBOs[0];
		VBO.uploadData(mesh.vertices);

		textureShader.setMatrix("model", modelMatrix);

		// set VBO to VAO's [0] binding index.
		glVertexArrayVertexBuffer(VAO, 0, VBOs[0].id(), 0, sizeof(Vertex));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Bind back to default FBO for ImGui to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Renderer::getMainFrameBufferTexture() const {
	return mainFrameBuffer.textureId();
}

Camera& Renderer::getCamera() {
	return camera;
}

Camera const& Renderer::getCamera() const {
	return camera;
}

void Renderer::setBlendMode(BlendingConfig configuration) {
	switch (configuration) {
		using enum BlendingConfig;
	case AlphaBlending:
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
		break;
	case AdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case PureAdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case PremultipliedAlpha:
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case Disabled:
		glDisable(GL_BLEND);
		break;
	}
}

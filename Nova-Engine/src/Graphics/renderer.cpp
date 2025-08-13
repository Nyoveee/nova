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
	assetManager		{ engine.assetManager },
	basicShader			{ "Assets/Shader/basic.vert", "Assets/Shader/basic.frag" },
	standardShader		{ "Assets/Shader/standard.vert", "Assets/Shader/basic.frag" },
	textureShader		{ "Assets/Shader/standard.vert", "Assets/Shader/image.frag" },
	VAO					{},
	EBO					{ 200000, BufferObject::Type::ElememtBuffer },
	camera				{},
	mainFrameBuffer		{ gameWidth, gameHeight }
{
	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);

	// construct a VBO. Allocate 50000 bytes of memory to this VBO.
	VBOs.push_back({ 200000, BufferObject::Type::VertexBuffer });

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

	glCreateVertexArrays(1, &VAO);

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(VAO, EBO.id());

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

	// Retrieve all game objects and prepare them for batch rendering..
	entt::registry& registry = engine.ecs.registry;

	for (auto&& [entity, transform, modelRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = assetManager.getAsset<Model>(modelRenderer.modelId);

		if (!model) {
			continue;
		}

		// Model matrix.
		glm::mat4x4 modelMatrix = glm::mat4x4{ 1 };
		modelMatrix = glm::translate(modelMatrix, transform.position);
		modelMatrix = glm::scale(modelMatrix, transform.scale / model->maxDimension);

		// Copy mesh to VBO..
		auto& VBO = VBOs[0];
		textureShader.setMatrix("model", modelMatrix);

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			// Get texture..
			auto iterator = modelRenderer.materials.find(mesh.materialName);

			if (iterator == modelRenderer.materials.end()) {
				std::cerr << "this shouldn't happen.";
				continue;
			}
			
			auto [texture, _] = assetManager.getAsset<Texture>(iterator->second.diffuseTextureId);

			if (!texture) {
				std::cerr << "Error retrieving asset!\n";
			}
			else {
				textureShader.setImageUniform("image", 0);
				glBindTextureUnit(0, texture->getTextureId());
			}

			VBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);

			// set VBO to VAO's [0] binding index.
			glVertexArrayVertexBuffer(VAO, 0, VBOs[0].id(), 0, sizeof(Vertex));

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
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

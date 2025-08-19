#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include "engine.h"
#include "renderer.h"
#include "window.h"
#include "vertex.h"
#include "ECS.h"
#include "assetManager.h"

#include <fstream>
#include "Component/component.h"

#undef max

constexpr GLuint clearValue = std::numeric_limits<GLuint>::max();
constexpr std::size_t colorIndex = 0;
constexpr std::size_t objectIdIndex = 1;

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine				{ engine },
	assetManager		{ engine.assetManager },
	basicShader			{ "System/Shader/basic.vert",		"System/Shader/basic.frag" },
	standardShader		{ "System/Shader/standard.vert",	"System/Shader/basic.frag" },
	textureShader		{ "System/Shader/standard.vert",	"System/Shader/image.frag" },
	blinnPhongShader	{ "System/Shader/blinnPhong.vert",	"System/Shader/blinnPhong.frag" },
	gridShader			{ "System/Shader/grid.vert",		"System/Shader/grid.frag" },
	outlineShader		{ "System/Shader/outline.vert",		"System/Shader/outline.frag" },
	VAO					{},
	EBO					{ 200000, BufferObject::Type::ElememtBuffer },
	camera				{},
	// main FBO shall contain color attachment 0 of vec4 and color attachment 1 of 32 byte int.
	// color attachment 0 is to store the resulting color, color attachment 1 is to store object id for object picking.
	mainFrameBuffer		{ gameWidth, gameHeight, { GL_RGBA16, GL_R32UI } }
{
	printOpenGLDriverDetails();

	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);

	// construct a VBO. Allocate some amount of bytes of memory to this VBO.
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
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureUnit));

	// enable attributes
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);

	// associate attribute 0 and 1 to binding index 0. 
	glVertexArrayAttribBinding(VAO, 0, bindingIndex);
	glVertexArrayAttribBinding(VAO, 1, bindingIndex);
	glVertexArrayAttribBinding(VAO, 2, bindingIndex);

	glEnable(GL_STENCIL_TEST);

	// sounds useless :rofl:
	glDisable(GL_DITHER);
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
}

DLL_API GLuint Renderer::getObjectId(glm::vec2 normalisedPosition) const {
	std::vector<GLuint> objectIds;
	objectIds.resize(1, 5);

	int xOffset = static_cast<int>(normalisedPosition.x * mainFrameBuffer.getWidth());	// x offset
	int yOffset = static_cast<int>(normalisedPosition.y * mainFrameBuffer.getHeight());	// y offset

	glGetTextureSubImage(
		mainFrameBuffer.textureIds()[objectIdIndex], 
		0,					// mipmap level (0 = base image)
		xOffset,			// x offset
		yOffset,			// y offset
		0,					// z offset
		1, 1, 1,			// width, height and depth of pixels to be read
		GL_RED_INTEGER,
		GL_UNSIGNED_INT, 
		sizeof(GLuint),		// size of pixels to be read
		objectIds.data()	// output parameter.
	);

	return objectIds[0];
}

void Renderer::update(float dt) {
	(void) dt;
}

void Renderer::render(RenderTarget target) {
	prepareRendering(target);

	renderModels();

	renderOutline();

	// Bind back to default FBO for ImGui to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<GLuint> const& Renderer::getMainFrameBufferTextures() const {
	return mainFrameBuffer.textureIds();
}

DLL_API void Renderer::enableWireframeMode(bool toEnable) const {
	if (toEnable) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

Camera& Renderer::getCamera() {
	return camera;
}

Camera const& Renderer::getCamera() const {
	return camera;
}

DLL_API void Renderer::recompileShaders() {
	blinnPhongShader.compile();
}

void Renderer::prepareRendering(RenderTarget target) {
	glEnable(GL_DEPTH_TEST);
	glStencilMask(0xFF);

	// set VBO to VAO's [0] binding index.
	glVertexArrayVertexBuffer(VAO, 0, VBOs[0].id(), 0, sizeof(Vertex));
	glBindVertexArray(VAO);

	// Clear default framebuffer regardless.
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	textureShader.use();

	// Bind to and clear main frame buffer if in used.
	switch (target)
	{
	case Renderer::RenderTarget::ToMainFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.fboId());

		constexpr GLfloat floatingClearValues[] = { 0.2f, 0.2f, 0.2f, 1.f };
		constexpr GLuint nullEntity = entt::null;
		constexpr GLfloat initialDepth = 1.f;
		constexpr GLint initialStencilValue = 0;

		glClearNamedFramebufferfv(mainFrameBuffer.fboId(), GL_COLOR, 0, floatingClearValues);
		glClearNamedFramebufferuiv(mainFrameBuffer.fboId(), GL_COLOR, 1, &nullEntity);
		glClearNamedFramebufferfi(mainFrameBuffer.fboId(), GL_DEPTH_STENCIL, 0, initialDepth, initialStencilValue);
		break;
	}
}

void Renderer::renderModels() {
	blinnPhongShader.use();
	blinnPhongShader.setMatrix("view", camera.view());
	blinnPhongShader.setMatrix("projection", camera.projection());
	// Retrieve all game objects and prepare them for batch rendering..
	entt::registry& registry = engine.ecs.registry;

	// preparing the stencil buffer for rendering outlines..
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// replaces the value in stencil buffer if both stencil buffer and depth buffer passed.
	glStencilFunc(GL_ALWAYS, 1, 0xFF);			// stencil test will always pass, mask of 0xFF means full byte comparison.

	auto& VBO = VBOs[0];

	blinnPhongShader.setVec3("cameraPos", camera.getPos());

	// STUB CODE!
	// just in case we have no light objects.
	blinnPhongShader.setVec3("lightPos", glm::vec3{ 0.f, 0.f, 0.f });
	blinnPhongShader.setVec3("lightColor", glm::vec3{ 1.f, 1.f, 1.f });

	// we need to set up light data..
	// FOR NOW WE WORK WITH ONE!
	for (auto&& [entity, transform, light] : registry.view<Transform, Light>().each()) {
		blinnPhongShader.setVec3("lightPos", transform.position);
		blinnPhongShader.setVec3("lightColor", light.color);
	}

	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = assetManager.getAsset<Model>(meshRenderer.modelId);

		if (!model) {
			continue;
		}

		blinnPhongShader.setMatrix("model", transform.modelMatrix);
		blinnPhongShader.setMatrix("normalMatrix", transform.normalMatrix);

		// Set up stencil operation
		if (meshRenderer.toRenderOutline) {
			glStencilMask(0xFF);	// allows writing the full byte of the stencil's buffer.
		}
		else {
			glStencilMask(0x00);	// don't write to the stencil buffer. this object is not going to be outlined.
		}

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			if (!setMaterial(blinnPhongShader, meshRenderer, mesh)) {
				continue;
			}

			VBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);

			blinnPhongShader.setUInt("objectId", static_cast<GLuint>(entity));
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void Renderer::renderOutline() {
	entt::registry& registry = engine.ecs.registry;

	// time to render the outlines..
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// we don't need to write to the stencil buffer anymore, we focus on testing..
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);	// test if the fragment of the outline is within the stencil buffer, discard if it is.
	glDisable(GL_DEPTH_TEST);				// i want to render my lines over other objects even if its in front.

	outlineShader.use();
	outlineShader.setMatrix("view", camera.view());
	outlineShader.setMatrix("projection", camera.projection());

	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		if (!meshRenderer.toRenderOutline) {
			continue;
		}

		auto [model, _] = assetManager.getAsset<Model>(meshRenderer.modelId);

		if (!model) {
			continue;
		}

		outlineShader.setMatrix("model", transform.modelMatrix);
		outlineShader.setVec3("color", { 255.f / 255.f, 136.f / 255.f, 0.f });

		for (auto const& mesh : model->meshes) {
			VBOs[0].uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

bool Renderer::setMaterial(Shader& shader, MeshRenderer const& meshRenderer, Model::Mesh const& mesh) {
	// Get material...
	auto iterator = meshRenderer.materials.find(mesh.materialName);

	if (iterator == meshRenderer.materials.end()) {
		std::cerr << "this shouldn't happen. material in component does not correspond to material on mesh.";
		return false;
	}
	
	// We have successfully retrieved the materials. Let's retrieve the individual components.
	auto&& [__, material] = *iterator;

	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, AssetID>) {
			auto&& [texture, result] = assetManager.getAsset<Texture>(albedo);

			if (!texture) {
				// fallback..
				shader.setBool("isUsingAlbedoMap", false);
				shader.setVec3("albedo", { 0.2f, 0.2f, 0.2f });
			}
			else {
				shader.setBool("isUsingAlbedoMap", true);
				glBindTextureUnit(0, texture->getTextureId());
				shader.setImageUniform("albedoMap", 0);
			}
		}
		else /* it's glm::vec3. */ {
			shader.setBool("isUsingAlbedoMap", false);
			shader.setVec3("albedo", albedo);
		}
		
	}, material.albedo);
	
	shader.setFloat("ambientFactor", material.ambient);
	//shader.setVec3();

	return true;
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

void Renderer::printOpenGLDriverDetails() const {
	GLubyte const* vendor = glGetString(GL_VENDOR);
	GLubyte const* renderer = glGetString(GL_RENDERER);
	GLubyte const* version = glGetString(GL_VERSION);
	GLubyte const* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	if (vendor) {
		std::cout << "OpenGL Vendor: " << vendor << std::endl;
	}
	if (renderer) {
		std::cout << "OpenGL Renderer: " << renderer << std::endl;
	}
	if (version) {
		std::cout << "OpenGL Version: " << version << std::endl;
	}
	if (glslVersion) {
		std::cout << "GLSL Version: " << glslVersion << std::endl;
	}
}

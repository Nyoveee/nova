#include <spdlog/spdlog.h>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include <glm/gtc/type_ptr.hpp>

#include "engine.h"
#include "renderer.h"
#include "window.h"
#include "vertex.h"
#include "ECS.h"
#include "assetManager.h"

#include <fstream>
#include "Component/component.h"
#include "Libraries/Profiling.h"

#undef max

constexpr GLuint clearValue = std::numeric_limits<GLuint>::max();
constexpr std::size_t colorIndex = 0;
constexpr std::size_t objectIdIndex = 1;

// 100 MB should be nothing right?
constexpr int AMOUNT_OF_MEMORY_ALLOCATED = 100000000;

// ok right?
constexpr int MAX_NUMBER_OF_LIGHT = 100;

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine				{ engine },
	assetManager		{ engine.assetManager },
	registry			{ engine.ecs.registry },
	basicShader			{ "System/Shader/basic.vert",		"System/Shader/basic.frag" },
	standardShader		{ "System/Shader/standard.vert",	"System/Shader/basic.frag" },
	textureShader		{ "System/Shader/standard.vert",	"System/Shader/image.frag" },
	colorShader			{ "System/Shader/standard.vert",	"System/Shader/color.frag" },
	blinnPhongShader	{ "System/Shader/blinnPhong.vert",	"System/Shader/blinnPhong.frag" },
	gridShader			{ "System/Shader/grid.vert",		"System/Shader/grid.frag" },
	outlineShader		{ "System/Shader/outline.vert",		"System/Shader/outline.frag" },
	VAO					{},
	VBO					{ AMOUNT_OF_MEMORY_ALLOCATED, BufferObject::Type::VertexBuffer },
	EBO					{ AMOUNT_OF_MEMORY_ALLOCATED, BufferObject::Type::ElememtBuffer },

						// we allocate the memory of all light data + space for 1 unsigned int indicating object count.
	LightSSBO			{ MAX_NUMBER_OF_LIGHT * sizeof(LightData) + alignof(LightData), BufferObject::Type::SSBO},

						// we allocate memory for view and projection matrix.
	SharedUBO			{ 2 * sizeof(glm::mat4), BufferObject::Type::UBO },
	camera				{},

	// main FBO shall contain color attachment 0 of vec4 and color attachment 1 of 32 byte int.
	// color attachment 0 is to store the resulting color, color attachment 1 is to store object id for object picking.
	mainFrameBuffer		{ gameWidth, gameHeight, { GL_RGBA16, GL_R32UI } }
{
	printOpenGLDriverDetails();

	// prepare the light SSBO. we bind light SSBO to binding point of 0.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, LightSSBO.id());

	// prepare the shared UBO. we bind UBO to binding point of 0.
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedUBO.id());

	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);
	glCreateVertexArrays(1, &VAO);

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(VAO, EBO.id());

	// for this VAO, associate bindingIndex 0 with this VBO. 
	GLuint bindingIndex = 0;
	glVertexArrayVertexBuffer(VAO, bindingIndex, VBO.id(), 0, sizeof(Vertex));

	// associate attribute index 0 and 1 with the respective attribute properties.
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureUnit));
	glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
	glVertexArrayAttribFormat(VAO, 4, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent));

	// enable attributes
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);
	glEnableVertexArrayAttrib(VAO, 3);
	glEnableVertexArrayAttrib(VAO, 4);

	// associate vertex attributes to binding index 0. 
	glVertexArrayAttribBinding(VAO, 0, bindingIndex);
	glVertexArrayAttribBinding(VAO, 1, bindingIndex);
	glVertexArrayAttribBinding(VAO, 2, bindingIndex);
	glVertexArrayAttribBinding(VAO, 3, bindingIndex);
	glVertexArrayAttribBinding(VAO, 4, bindingIndex);

	glEnable(GL_STENCIL_TEST);	

	// sounds useless :rofl: (it's interfering with my MRT attempt)
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
	ZoneScoped;
	(void) dt;
}

void Renderer::render(RenderTarget target) {
	ZoneScoped;
	prepareRendering(target);

	renderModels();

	//renderOutline();

	// Bind back to default FBO for ImGui to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<GLuint> const& Renderer::getMainFrameBufferTextures() const {
	return mainFrameBuffer.textureIds();
}

void Renderer::enableWireframeMode(bool toEnable) const {
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
	ZoneScopedC(tracy::Color::PaleVioletRed1);
	// =================================================================
	// Configure pre rendering settings
	// =================================================================

	// of course.
	glEnable(GL_DEPTH_TEST);
	glStencilMask(0xFF);

	// set VBO to VAO's [0] binding index.
	glVertexArrayVertexBuffer(VAO, 0, VBO.id(), 0, sizeof(Vertex));
	glBindVertexArray(VAO);

	// =================================================================
	// Clear frame buffers.
	// =================================================================
	
	// Clear default framebuffer.
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// For some reason, for framebuffers with multiple color attachments
	// we need to bind to a shader that writes to all the other color attachments
	// even though clear operation does not use our shader at all
	// seems to be a differing implementation for NVIDIA GPUs..
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

	// =================================================================
	// Set up the uniforms for my respective shaders
	// @TODO: Use a UBO to share common uniforms across all shaders like view and projection.
	// Note: calling shader.use() before setting uniforms is redundant because we are using DSA.
	// =================================================================
	// Shared across all shaders. We use a shared UBO for this.
	glNamedBufferSubData(SharedUBO.id(), 0, sizeof(glm::mat4x4), glm::value_ptr(camera.view()));
	glNamedBufferSubData(SharedUBO.id(), sizeof(glm::mat4x4), sizeof(glm::mat4x4), glm::value_ptr(camera.projection())); // offset bcuz 2nd data member.

	// ==== Blinn Phong ====
	blinnPhongShader.setVec3("cameraPos", camera.getPos());

	// we need to set up light data..
	std::array<LightData, MAX_NUMBER_OF_LIGHT> lightData;

	unsigned int numOfLights = 0;
	for (auto&& [entity, transform, light] : registry.view<Transform, Light>().each()) {
		if (numOfLights == MAX_NUMBER_OF_LIGHT) {
			spdlog::warn("Max number of lights reached!");
			break;
		}

		lightData[numOfLights] = { transform.position, glm::vec3{ light.color } * light.intensity, static_cast<unsigned int>(light.type) };
		++numOfLights;
	}

	// Send it over to SSBO.
	glNamedBufferSubData(LightSSBO.id(), 0, sizeof(unsigned int), &numOfLights);	// copy the unsigned int representing number of lights into SSBO.

	// copy all the light data to the SSBO.
	// offset is an alignment of LightData!! because of alignment requirements of this struct!
	// omdayz..
	glNamedBufferSubData(LightSSBO.id(), alignof(LightData), numOfLights * sizeof(LightData), lightData.data());

	// ==== Color shader ====
	// ..
	
	// ==== Texture shader ====
	// ..

	// ==== Outline Shader ====
	// ..
}

void Renderer::renderModels() {
	ZoneScopedC(tracy::Color::PaleVioletRed1);	
	// preparing the stencil buffer for rendering outlines..
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// replaces the value in stencil buffer if both stencil buffer and depth buffer passed.
	glStencilFunc(GL_ALWAYS, 1, 0xFF);			// stencil test will always pass, mask of 0xFF means full byte comparison.

	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = assetManager.getAsset<Model>(meshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		setModelUniforms(transform, entity);

		// Set up stencil operation
		if (meshRenderer.toRenderOutline) {
			glStencilMask(0xFF);	// allows writing the full byte of the stencil's buffer.
		}
		else {
			glStencilMask(0x00);	// don't write to the stencil buffer. this object is not going to be outlined.
		}

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			Material const* material = obtainMaterial(meshRenderer, mesh);

			if (!material) {
				continue;
			}

			// uses the appropriate shader and sets the appropriate uniform based on
			// configured rendering pipeline.
			switch (material->renderingPipeline)
			{
			case Material::Pipeline::PBR:	
				setupBlinnPhongShader(*material);
				break;
			case Material::Pipeline::BlinnPhong:
				setupBlinnPhongShader(*material);
				break;
			case Material::Pipeline::Color:
				setupColorShader(*material);
			}

			// time to draw!
			VBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void Renderer::renderOutline() {
	// time to render the outlines..
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	// we don't need to write to the stencil buffer anymore, we focus on testing..
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);	// test if the fragment of the outline is within the stencil buffer, discard if it is.
	glDisable(GL_DEPTH_TEST);				// i want to render my lines over other objects even if its in front.

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
			VBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void Renderer::setupBlinnPhongShader(Material const& material) {
	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, AssetID>) {
			auto&& [texture, result] = assetManager.getAsset<Texture>(albedo);

			if (!texture) {
				// fallback..
				blinnPhongShader.setBool("isUsingAlbedoMap", false);
				blinnPhongShader.setVec3("albedo", { 0.2f, 0.2f, 0.2f });
			}
			else {
				blinnPhongShader.setBool("isUsingAlbedoMap", true);
				glBindTextureUnit(0, texture->getTextureId());
				blinnPhongShader.setImageUniform("albedoMap", 0);
			}
		}
		else /* it's Color */ {
			blinnPhongShader.setBool("isUsingAlbedoMap", false);
			blinnPhongShader.setVec3("albedo", albedo);
		}

	}, material.albedo);

	// Handle normal map
	if (material.normalMap) {
		auto&& [normalMap, result] = assetManager.getAsset<Texture>(material.normalMap.value());

		if (normalMap) {
			blinnPhongShader.setBool("isUsingNormalMap", true);
			glBindTextureUnit(1, normalMap->getTextureId());
			blinnPhongShader.setImageUniform("normalMap", 1);
		}
	}
	else {
		blinnPhongShader.setBool("isUsingNormalMap", false);
	}

	blinnPhongShader.setFloat("ambientFactor", material.ambient);

	// calling use after setting uniforms?
	// thats right, DSA the goat.
	blinnPhongShader.use();
}

void Renderer::setupPBRShader(Material const& material) {
	(void) material;
}

void Renderer::setupColorShader(Material const& material) {
	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, AssetID>) {
			auto&& [texture, result] = assetManager.getAsset<Texture>(albedo);

			if (!texture) {
				// fallback..
				colorShader.use();
				colorShader.setVec3("color", { 0.2f, 0.2f, 0.2f });
			}
			else {
				textureShader.use();
				glBindTextureUnit(0, texture->getTextureId());
				textureShader.setImageUniform("image", 0);
			}
		}
		else /* it's Color */ {
			colorShader.use();
			colorShader.setVec3("color", albedo);
		}

	}, material.albedo);
}

void Renderer::setModelUniforms(Transform const& transform, entt::entity entity) {
	blinnPhongShader.setMatrix("model", transform.modelMatrix);
	blinnPhongShader.setMatrix("normalMatrix", transform.normalMatrix);
	blinnPhongShader.setUInt("objectId", static_cast<GLuint>(entity));

	colorShader.setMatrix("model", transform.modelMatrix);
	colorShader.setUInt("objectId", static_cast<GLuint>(entity));

	textureShader.setMatrix("model", transform.modelMatrix);
	textureShader.setUInt("objectId", static_cast<GLuint>(entity));
}

Material const* Renderer::obtainMaterial(MeshRenderer const& meshRenderer, Model::Mesh const& mesh) {
	auto iterator = meshRenderer.materials.find(mesh.materialName);

	if (iterator == meshRenderer.materials.end()) {
		spdlog::warn("This shouldn't happen. Material in component does not correspond to material on mesh.");
		return nullptr;
	}

	// We have successfully retrieved the materials. Let's retrieve the individual components.
	auto&& [__, material] = *iterator;
	return &material;
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
		spdlog::info("OpenGL Vendor: {}", reinterpret_cast<const char*>(vendor));
	}
	if (renderer) {
		spdlog::info("OpenGL Renderer: {}", reinterpret_cast<const char*>(renderer));
	}
	if (version) {
		spdlog::info("OpenGL Version: {}", reinterpret_cast<const char*>(version));
	}
	if (glslVersion) {
		spdlog::info("GLSL Version: {}", reinterpret_cast<const char*>(glslVersion));
	}
}

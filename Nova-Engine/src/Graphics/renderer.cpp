#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

#include <glm/gtc/type_ptr.hpp>

#include "Engine/engine.h"
#include "renderer.h"
#include "Engine/window.h"
#include "vertex.h"
#include "Component/ECS.h"
#include "ResourceManager/resourceManager.h"

#include <fstream>
#include "Component/component.h"
#include "Profiling.h"
#include "Logger.h"

#undef max

constexpr GLuint clearValue = std::numeric_limits<GLuint>::max();

// 100 MB should be nothing right?
constexpr int AMOUNT_OF_MEMORY_ALLOCATED = 100 * 1024 * 1024;

// we allow a maximum of 10,000 triangle. (honestly some arbritary value lmao)
constexpr int MAX_DEBUG_TRIANGLES = 10000;
constexpr int AMOUNT_OF_MEMORY_FOR_DEBUG = MAX_DEBUG_TRIANGLES * 3 * sizeof(SimpleVertex);

// ok right?
constexpr int MAX_NUMBER_OF_LIGHT = 100;

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine				{ engine },
	resourceManager		{ engine.resourceManager },
	registry			{ engine.ecs.registry },
	basicShader			{ "System/Shader/basic.vert",				"System/Shader/basic.frag" },
	standardShader		{ "System/Shader/standard.vert",			"System/Shader/basic.frag" },
	textureShader		{ "System/Shader/standard.vert",			"System/Shader/image.frag" },
	colorShader			{ "System/Shader/standard.vert",			"System/Shader/color.frag" },
	blinnPhongShader	{ "System/Shader/blinnPhong.vert",			"System/Shader/blinnPhong.frag" },
	gridShader			{ "System/Shader/grid.vert",				"System/Shader/grid.frag" },
	outlineShader		{ "System/Shader/outline.vert",				"System/Shader/outline.frag" },
	debugShader			{ "System/Shader/debug.vert",				"System/Shader/debug.frag" },
	debugOverlayShader	{ "System/Shader/squareOverlay.vert",		"System/Shader/debugOverlay.frag" },
	objectIdShader		{ "System/Shader/standard.vert",			"System/Shader/objectId.frag" },
	skyboxShader		{ "System/Shader/skybox.vert",				"System/Shader/skybox.frag" },
	mainVAO				{},
	debugPhysicsVAO		{},
	mainVBO				{ AMOUNT_OF_MEMORY_ALLOCATED },
	debugPhysicsVBO		{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	EBO					{ AMOUNT_OF_MEMORY_ALLOCATED },

						// we allocate the memory of all light data + space for 1 unsigned int indicating object count.
	pointLightSSBO		{ MAX_NUMBER_OF_LIGHT * sizeof(PointLightData) + alignof(PointLightData)},
	directionalLightSSBO{ MAX_NUMBER_OF_LIGHT * sizeof(DirectionalLightData) + alignof(DirectionalLightData)},
	spotLightSSBO		{ MAX_NUMBER_OF_LIGHT * sizeof(SpotLightData) + alignof(SpotLightData)},

						// we allocate memory for view and projection matrix.
	sharedUBO			{ 2 * sizeof(glm::mat4) },
	camera				{},
	numOfDebugTriangles	{},
	isOnWireframeMode	{},

	mainFrameBuffer			{ gameWidth, gameHeight, { GL_RGBA16 } },
	physicsDebugFrameBuffer { gameWidth, gameHeight, { GL_RGBA8 } },
	objectIdFrameBuffer		{ gameWidth, gameHeight, { GL_R32UI } }
{
	printOpenGLDriverDetails();

	glLineWidth(2.f);

	// ======================================================
	// Prepare shared UBO, that will be used by all shaders. (like view and projection matrix.)
	// ======================================================
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedUBO.id());

	// prepare the light SSBOs. we bind light SSBO to binding point of 0.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointLightSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, directionalLightSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spotLightSSBO.id());

	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);

	// ======================================================
	// Main VAO configuration
	// - Bind the proper EBO
	// - Define and configure the proper vertex attributes
	// - Bind the proper VBO to	these vertex attributes
	// ======================================================
	glCreateVertexArrays(1, &mainVAO);

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(mainVAO, EBO.id());

	// for this VAO, associate bindingIndex 0 with this VBO. 
	constexpr GLuint bindingIndex = 0;
	glVertexArrayVertexBuffer(mainVAO, bindingIndex, mainVBO.id(), 0, sizeof(Vertex));

	// associate attribute index 0 and 1 with the respective attribute properties.
	glVertexArrayAttribFormat(mainVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribFormat(mainVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureUnit));
	glVertexArrayAttribFormat(mainVAO, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribFormat(mainVAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
	glVertexArrayAttribFormat(mainVAO, 4, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent));

	// enable attributes
	glEnableVertexArrayAttrib(mainVAO, 0);
	glEnableVertexArrayAttrib(mainVAO, 1);
	glEnableVertexArrayAttrib(mainVAO, 2);
	glEnableVertexArrayAttrib(mainVAO, 3);
	glEnableVertexArrayAttrib(mainVAO, 4);

	// associate vertex attributes to binding index 0. 
	glVertexArrayAttribBinding(mainVAO, 0, bindingIndex);
	glVertexArrayAttribBinding(mainVAO, 1, bindingIndex);
	glVertexArrayAttribBinding(mainVAO, 2, bindingIndex);
	glVertexArrayAttribBinding(mainVAO, 3, bindingIndex);
	glVertexArrayAttribBinding(mainVAO, 4, bindingIndex);

	// ======================================================
	// Debug Physics VAO configuration
	// - No EBO. A much simpler VAO containing only position.
	// ======================================================
	glCreateVertexArrays(1, &debugPhysicsVAO);
	
	// for this VAO, associate bindingIndex 1 with this VBO. 
	constexpr GLuint debugBindingIndex = 1;

	glVertexArrayVertexBuffer(debugPhysicsVAO, debugBindingIndex, debugPhysicsVBO.id(), 0, sizeof(SimpleVertex));

	// associate attribute index 0 with the respective attribute properties.
	glVertexArrayAttribFormat(debugPhysicsVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(SimpleVertex, pos));

	// enable attribute
	glEnableVertexArrayAttrib(debugPhysicsVAO, 0);

	// associate vertex attribute 0 with binding index 1.
	glVertexArrayAttribBinding(debugPhysicsVAO, 0, debugBindingIndex);

	//allCameras.push_back(&camera);
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &mainVAO);
}

DLL_API GLuint Renderer::getObjectId(glm::vec2 normalisedPosition) const {
	GLuint objectId;

	int xOffset = static_cast<int>(normalisedPosition.x * objectIdFrameBuffer.getWidth());	// x offset
	int yOffset = static_cast<int>(normalisedPosition.y * objectIdFrameBuffer.getHeight());	// y offset

	glGetTextureSubImage(
		objectIdFrameBuffer.textureIds()[0], 
		0,					// mipmap level (0 = base image)
		xOffset,			// x offset
		yOffset,			// y offset
		0,					// z offset
		1, 1, 1,			// width, height and depth of pixels to be read
		GL_RED_INTEGER,
		GL_UNSIGNED_INT, 
		sizeof(GLuint),		// size of pixels to be read
		&objectId			// output parameter.
	);

	return objectId;
}

void Renderer::update(float dt) {
	ZoneScoped;
	(void) dt;
}

void Renderer::render(RenderTarget target, bool toRenderDebug) {
	ZoneScoped;
	prepareRendering(target);

	renderSkyBox();

	renderModels();

	//renderOutline();

	if (toRenderDebug) {
		debugRender();
	}

	// Bind back to default FBO for ImGui to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<GLuint> const& Renderer::getMainFrameBufferTextures() const {
	return mainFrameBuffer.textureIds();
}

void Renderer::enableWireframeMode(bool toEnable) {
	if (toEnable) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		isOnWireframeMode = toEnable;
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		isOnWireframeMode = toEnable;
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
	skyboxShader.compile();
}

void Renderer::debugRender() {
	// ================================================
	// 1. We first render all debug shapes (triangles and lines) into a separate FBO
	// ================================================
	glBindVertexArray(debugPhysicsVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, physicsDebugFrameBuffer.fboId());

	// Clear physics debug framebuffer.
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// enable wireframe mode only for debug overlay.
	if (!isOnWireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	debugShader.use();
	debugShader.setVec4("color", { 0.f, 1.f, 0.f, 1.f });
	glDrawArrays(GL_TRIANGLES, 0, numOfDebugTriangles * 3);

	glDisable(GL_DEPTH_TEST);
	debugOverlayShader.use();
	numOfDebugTriangles = 0;

	// ================================================
	// 2. We overlay this resulting debug shapes into our main FBO, with alpha blending.
	// (so post processing)
	// ================================================

	// disable wireframe mode, restoring to normal fill
	if (!isOnWireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	setBlendMode(BlendingConfig::AlphaBlending);
	glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.fboId());
	
	// set image uniform accordingly..
	glBindTextureUnit(0, physicsDebugFrameBuffer.textureIds()[0]);
	debugOverlayShader.setImageUniform("debugOverlay", 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
}

void Renderer::submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3, ColorA color) {
	(void) color;

	if (numOfDebugTriangles > MAX_DEBUG_TRIANGLES) {
		std::cerr << "too much triangles!\n";
		return;
	}

	debugPhysicsVBO.uploadData(std::vector<SimpleVertex>{ { vertice1 }, { vertice2 }, { vertice3 } }, 3 * numOfDebugTriangles * sizeof(SimpleVertex));
	++numOfDebugTriangles;
}

void Renderer::prepareRendering(RenderTarget target) {
	(void) target;

	ZoneScopedC(tracy::Color::PaleVioletRed1);
	// =================================================================
	// Configure pre rendering settings
	// =================================================================
	
	// of course.
	glEnable(GL_DEPTH_TEST);
	glStencilMask(0xFF);
	glDisable(GL_BLEND);

	// set VBO to VAO's [0] binding index and bind to main VAO.
	glVertexArrayVertexBuffer(mainVAO, 0, mainVBO.id(), 0, sizeof(Vertex));
	glBindVertexArray(mainVAO);

	// =================================================================
	// Clear frame buffers.
	// =================================================================
	
	// Clear default framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear main framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.fboId());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	// Clear object id framebuffer.

	// For some reason, for framebuffers with integer color attachments
	// we need to bind to a shader that writes to integer output
	// even though clear operation does not use our shader at all
	// seems to be a differing / conflicting implementation for NVIDIA GPUs..
	objectIdShader.use();

	constexpr GLuint	nullEntity			= entt::null;
	constexpr GLfloat	initialDepth		= 1.f;
	constexpr GLint		initialStencilValue = 0;

	glDisable(GL_DITHER);
	glClearNamedFramebufferuiv(objectIdFrameBuffer.fboId(), GL_COLOR, 0, &nullEntity);
	glClearNamedFramebufferfi(objectIdFrameBuffer.fboId(), GL_DEPTH_STENCIL, 0, initialDepth, initialStencilValue);
	glEnable(GL_DITHER);

	// =================================================================
	// Set up the uniforms for my respective shaders
	// Note: calling shader.use() before setting uniforms is redundant because we are using DSA.
	// =================================================================
	// Shared across all shaders. We use a shared UBO for this.
	glNamedBufferSubData(sharedUBO.id(), 0, sizeof(glm::mat4x4), glm::value_ptr(camera.view()));
	glNamedBufferSubData(sharedUBO.id(), sizeof(glm::mat4x4), sizeof(glm::mat4x4), glm::value_ptr(camera.projection())); // offset bcuz 2nd data member.

	// ==== Blinn Phong ====
	blinnPhongShader.setVec3("cameraPos", camera.getPos());

	// we need to set up light data..
	std::array<PointLightData, MAX_NUMBER_OF_LIGHT>			pointLightData;
	std::array<DirectionalLightData, MAX_NUMBER_OF_LIGHT>	directionalLightData;
	std::array<SpotLightData, MAX_NUMBER_OF_LIGHT>			spotLightData;

	unsigned int numOfPtLights = 0;
	unsigned int numOfDirLights = 0;
	unsigned int numOfSpotLights = 0;
	for (auto&& [entity, transform, light] : registry.view<Transform, Light>().each()) {
		switch (light.type)
		{
		case Light::Type::PointLight:
			if (numOfPtLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Max number of point lights reached!");
				continue;
			}
			pointLightData[numOfPtLights++] = {
				transform.position,
				glm::vec3{ light.color } * light.intensity,
				light.attenuation
			};
			break;

		case Light::Type::Directional:
		{
			if (numOfDirLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Max number of directional lights reached!");
				continue;
			}
			glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
			directionalLightData[numOfDirLights++] = {
				glm::normalize(forward),
				glm::vec3{ light.color } *light.intensity
			};
			break;
		}

		case Light::Type::Spotlight:
		{
			if (numOfSpotLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Max number of spot lights reached!");
				continue;
			}
			glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
			spotLightData[numOfSpotLights++] = {
				transform.position,
				glm::normalize(forward),
				glm::vec3{ light.color } *light.intensity,
				light.attenuation,
				light.cutOffAngle,
				light.outerCutOffAngle
			};
			break;
		}

		}
	}

	// Send it over to SSBO.
	glNamedBufferSubData(pointLightSSBO.id(), 0, sizeof(unsigned int), &numOfPtLights);	// copy the unsigned int representing number of lights into SSBO.
	glNamedBufferSubData(directionalLightSSBO.id(), 0, sizeof(unsigned int), &numOfDirLights);
	glNamedBufferSubData(spotLightSSBO.id(), 0, sizeof(unsigned int), &numOfSpotLights);

	// copy all the light data to the SSBO.
	// offset is an alignment of LightData!! because of alignment requirements of this struct!
	// omdayz..
	glNamedBufferSubData(pointLightSSBO.id(), alignof(PointLightData), numOfPtLights * sizeof(PointLightData), pointLightData.data());
	glNamedBufferSubData(directionalLightSSBO.id(), alignof(DirectionalLightData), numOfDirLights * sizeof(DirectionalLightData), directionalLightData.data());
	glNamedBufferSubData(spotLightSSBO.id(), alignof(SpotLightData), numOfSpotLights * sizeof(SpotLightData), spotLightData.data());

	// ==== Color shader ====
	// ..
	
	// ==== Texture shader ====
	// ..

	// ==== Outline Shader ====
	// ..
}

void Renderer::renderSkyBox() {
	glDisable(GL_DEPTH_TEST);

	for (auto&& [entityId, skyBox] : registry.view<SkyBox>().each()) {
		auto [asset, status] = resourceManager.getResource<CubeMap>(skyBox.cubeMapId);

		// skybox not loaded..
		if (!asset) {
			continue;
		}

		skyboxShader.use();
		skyboxShader.setImageUniform("equirectangularMap", 0);
		glBindTextureUnit(0, asset->getTextureId());
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// only render the very first skybox.
		return;
	}
}

void Renderer::renderModels() {
	ZoneScopedC(tracy::Color::PaleVioletRed1);	

	// enable back face culling for our 3d models..
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	// preparing the stencil buffer for rendering outlines..
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// replaces the value in stencil buffer if both stencil buffer and depth buffer passed.
	glStencilFunc(GL_ALWAYS, 1, 0xFF);			// stencil test will always pass, mask of 0xFF means full byte comparison.

	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);

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
			mainVBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);

			// render object id into object id FBO.
			renderObjectId(mesh.numOfTriangles * 3);
		}
	}

	glDisable(GL_CULL_FACE);
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

		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);

		if (!model) {
			continue;
		}

		outlineShader.setMatrix("model", transform.modelMatrix);
		outlineShader.setVec3("color", { 255.f / 255.f, 136.f / 255.f, 0.f });

		for (auto const& mesh : model->meshes) {
			mainVBO.uploadData(mesh.vertices);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void Renderer::renderObjectId(GLsizei count) {
	glDisable(GL_DITHER);

	glBindFramebuffer(GL_FRAMEBUFFER, objectIdFrameBuffer.fboId());
	objectIdShader.use();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.fboId());

	glEnable(GL_DITHER);
}

void Renderer::setupBlinnPhongShader(Material const& material) {
	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(albedo);

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
		auto&& [normalMap, result] = resourceManager.getResource<Texture>(material.normalMap.value());

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

		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(albedo);

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

	colorShader.setMatrix("model", transform.modelMatrix);

	textureShader.setMatrix("model", transform.modelMatrix);

	objectIdShader.setMatrix("model", transform.modelMatrix);
	objectIdShader.setUInt("objectId", static_cast<GLuint>(entity));
}

Material const* Renderer::obtainMaterial(MeshRenderer const& meshRenderer, Model::Mesh const& mesh) {
	auto iterator = meshRenderer.materials.find(mesh.materialName);

	if (iterator == meshRenderer.materials.end()) {
		Logger::warn("This shouldn't happen. Material in component does not correspond to material on mesh.");
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
		Logger::info("OpenGL Vendor: {}", reinterpret_cast<const char*>(vendor));
	}
	if (renderer) {
		Logger::info("OpenGL Renderer: {}", reinterpret_cast<const char*>(renderer));
	}
	if (version) {
		Logger::info("OpenGL Version: {}", reinterpret_cast<const char*>(version));
	}
	if (glslVersion) {
		Logger::info("GLSL Version: {}", reinterpret_cast<const char*>(glslVersion));
	}
}

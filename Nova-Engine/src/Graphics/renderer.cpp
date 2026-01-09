#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "Engine/engine.h"
#include "renderer.h"
#include "Engine/window.h"
#include "vertex.h"
#include "ECS/ECS.h"
#include "ResourceManager/resourceManager.h"
#include "DebugShapes.h"
#include "component.h"
#include "Profiling.h"
#include "Logger.h"

#include "RandomRange.h"
#include "systemResource.h"

#undef max

namespace {
	glm::vec3 findMaxBound(glm::vec3 boundOne) {
		return boundOne;
	}

	template <typename ...Args>
	glm::vec3 findMaxBound(glm::vec3 bound, Args... bounds) {
		return glm::max(bound, findMaxBound(bounds...));
	}

	glm::vec3 findMinBound(glm::vec3 boundOne) {
		return boundOne;
	}

	template <typename ...Args>
	glm::vec3 findMinBound(glm::vec3 bound, Args... bounds) {
		return glm::min(bound, findMinBound(bounds...));
	}

	// INVALID_ID means it's not holding to any dynamically allocated resource.
	constexpr inline GLuint INVALID_ID = std::numeric_limits<GLuint>::max();
}

constexpr ColorA whiteColor{ 1.f, 1.f, 1.f, 1.f };

constexpr GLuint clearValue = std::numeric_limits<GLuint>::max();

// 100 MB should be nothing right?
constexpr int AMOUNT_OF_MEMORY_ALLOCATED = 100 * 1024 * 1024;

// we allow a maximum of 10,000 triangle. (honestly some arbritary value lmao)
constexpr int MAX_DEBUG_TRIANGLES = 100000;
constexpr int MAX_DEBUG_LINES	  = 100000;
constexpr int AMOUNT_OF_MEMORY_FOR_DEBUG = MAX_DEBUG_TRIANGLES * 3 * sizeof(glm::vec3);

// ok right?
constexpr int MAX_NUMBER_OF_LIGHT = 250;

constexpr int MAX_NUMBER_OF_BONES = 250;

// For cluster forward rendering..
constexpr unsigned int gridSizeX = 16;
constexpr unsigned int gridSizeY = 9;
constexpr unsigned int gridSizeZ = 24;
constexpr unsigned int numClusters = gridSizeX * gridSizeY * gridSizeZ;

// Shadow mapping..
constexpr int SHADOW_MAP_WIDTH  = 2048;
constexpr int SHADOW_MAP_HEIGHT = 2048;

#pragma warning( push )
#pragma warning(disable : 4324)			// disable warning about structure being padded, that's exactly what i wanted.

struct alignas(16) Cluster {
	glm::vec4 minPoint;
	glm::vec4 maxPoint;
	unsigned int pointLightCount;
	unsigned int spotLightCount;
	unsigned int pointLightIndices[25];
	unsigned int spotLightIndices[25];
};

#pragma warning( pop )

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine							{ engine },
	gameWidth						{ gameWidth },
	gameHeight						{ gameHeight },
	resourceManager					{ engine.resourceManager },
	registry						{ engine.ecs.registry },
	basicShader						{ "System/Shader/basic.vert",						"System/Shader/basic.frag" },
	standardShader					{ "System/Shader/standard.vert",					"System/Shader/basic.frag" },
	textureShader					{ "System/Shader/standard.vert",					"System/Shader/image.frag" },
	colorShader						{ "System/Shader/standard.vert",					"System/Shader/color.frag" },
	bloomDownSampleShader			{ "System/Shader/squareOverlay.vert",				"System/Shader/bloomDownSample.frag" },
	bloomUpSampleShader				{ "System/Shader/squareOverlay.vert",				"System/Shader/bloomUpSample.frag" },
	bloomFinalShader				{ "System/Shader/squareOverlay.vert",				"System/Shader/bloomFinal.frag" },
	postprocessingShader			{ "System/Shader/squareOverlay.vert",				"System/Shader/postprocessing.frag" },
	gridShader						{ "System/Shader/grid.vert",						"System/Shader/grid.frag" },
	outlineShader					{ "System/Shader/outline.vert",						"System/Shader/outline.frag" },
	debugShader						{ "System/Shader/debug.vert",						"System/Shader/debug.frag" },
	overlayShader					{ "System/Shader/squareOverlay.vert",				"System/Shader/overlay.frag" },
	objectIdShader					{ "System/Shader/standard.vert",					"System/Shader/objectId.frag" },
	uiImageObjectIdShader			{ "System/Shader/texture2D.vert",					"System/Shader/objectId.frag" },
	uiTextObjectIdShader			{ "System/Shader/text.vert",						"System/Shader/objectId.frag" },
	skyboxShader					{ "System/Shader/skybox.vert",						"System/Shader/skybox.frag" },
	toneMappingShader				{ "System/Shader/squareOverlay.vert",				"System/Shader/tonemap.frag" },
	particleShader					{ "System/Shader/ParticleSystem/particle.vert",     "System/Shader/ParticleSystem/particle.frag"},
	textShader						{ "System/Shader/text.vert",						"System/Shader/text.frag"},
	texture2dShader					{ "System/Shader/texture2d.vert",					"System/Shader/image2D.frag"},
	shadowMapShader					{ "System/Shader/shadow.vert",						"System/Shader/empty.frag" },
	depthGBufferShader				{ "System/Shader/gbuffer.vert",						"System/Shader/gbuffer.frag" },
	ssaoShader						{ "System/Shader/squareOverlay.vert",				"System/Shader/ssaoGeneration.frag" },
	clusterBuildingCompute			{ "System/Shader/clusterBuilding.compute" },
	clusterLightCompute				{ "System/Shader/clusterLightAssignment.compute" },
	mainVAO							{},
	positionsVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	textureCoordinatesVBO			{ AMOUNT_OF_MEMORY_ALLOCATED },
	normalsVBO						{ AMOUNT_OF_MEMORY_ALLOCATED },
	tangentsVBO						{ AMOUNT_OF_MEMORY_ALLOCATED },
	skeletalVBO						{ AMOUNT_OF_MEMORY_ALLOCATED },
	debugPhysicsVBO					{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugPhysicsLineVBO				{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugNavMeshVBO					{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugParticleShapeVBO			{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	textVBO							{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	EBO								{ AMOUNT_OF_MEMORY_ALLOCATED },

	gameLights						{ MAX_NUMBER_OF_LIGHT },
	editorLights					{ MAX_NUMBER_OF_LIGHT },
	gameClusterSSBO					{ numClusters * sizeof(Cluster) },
	editorClusterSSBO				{ numClusters * sizeof(Cluster) },

									// we allocate memory for view and projection matrix, view * projection matrix, and 64 ssao sample kernels.
	sharedUBO						{ 3 * sizeof(glm::mat4) + 64 * sizeof(glm::vec4) },
	
									// we allocate the memory of all bone data + space for 1 unsigned int indicating true or false (whether the current invocation is a skinned meshrenderer).
	bonesSSBO						{ MAX_NUMBER_OF_BONES * sizeof(glm::mat4x4) + alignof(glm::vec4) },
	editorCamera					{},
	gameCamera						{},
	numOfPhysicsDebugTriangles		{},
	numOfNavMeshDebugTriangles		{},
	isOnWireframeMode				{},
	hasDirectionalLightShadowCaster {},
	directionalLightViewMatrix		{},
	directionalLightDir				{},
	timeElapsed						{},
	ssaoNoiseTextureId				{ INVALID_ID },
	hdrExposure						{ 0.9f },
	toneMappingMethod				{ ToneMappingMethod::ACES },
															 // main		normal
	editorMainFrameBuffer			{ gameWidth, gameHeight, { GL_RGBA16F,	GL_RGB8_SNORM } },
	gameMainFrameBuffer				{ gameWidth, gameHeight, { GL_RGBA16F,	GL_RGB8_SNORM } },
	uiMainFrameBuffer				{ gameWidth, gameHeight, { GL_RGBA8 } },
	physicsDebugFrameBuffer			{ gameWidth, gameHeight, { GL_RGBA8 } },
	objectIdFrameBuffer				{ gameWidth, gameHeight, { GL_R32UI } },
	uiObjectIdFrameBuffer			{ gameWidth, gameHeight, { GL_R32UI } },
	bloomFrameBuffer				{ gameWidth, gameHeight, 5 },
	ssaoFrameBuffer					{ gameWidth, gameHeight, { GL_R8 } },
	directionalLightShadowFBO		{ SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT },
	toGammaCorrect					{ true },
	toPostProcess					{ false },
	UIProjection					{ glm::ortho(0.0f, static_cast<float>(gameWidth), 0.0f, static_cast<float>(gameHeight)) }
{
	randomiseChromaticAberrationoffset();

	printOpenGLDriverDetails();

	glLineWidth(2.f);

	// ======================================================
	// Prepare shared UBO, that will be used by all shaders. (like view and projection matrix.)
	// ======================================================
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedUBO.id());

	// we bind bones SSBO to 3.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bonesSSBO.id());

	// Set the correct viewport
	glViewport(0, 0, gameWidth, gameHeight);

	// ======================================================
	// Main VAO configuration
	// - Bind the proper EBO
	// - Define and configure the proper vertex attributes
	// - Bind the proper VBO to	these vertex attributes
	// 
	// - vertex attribute 0 -> positions			-> binding index 0 -> positionsVBO
	// - vertex attribute 1 -> textureCoordinates	-> binding index 1 -> textureCoordinatesVBO
	// - vertex attribute 2 -> normals				-> binding index 2 -> normalsVBO
	// - vertex attribute 3 -> tangents				-> binding index 3 -> tangentsVBO
	// - vertex attribute 4 -> boneIndices[4]		-> binding index 4 -> skeletalVBO
	// - vertex attribute 5 -> weights[4]			-> binding index 4 -> skeletalVBO
	// ======================================================
	glCreateVertexArrays(1, &mainVAO);

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(mainVAO, EBO.id());

	// specify vertex attribute and VBO bindings..
	glVertexArrayAttribFormat(mainVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(mainVAO, 0, positionsVBO.id(), 0, sizeof(glm::vec3));

	glVertexArrayAttribFormat(mainVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(mainVAO, 1, textureCoordinatesVBO.id(), 0, sizeof(glm::vec2));
	
	glVertexArrayAttribFormat(mainVAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(mainVAO, 2, normalsVBO.id(), 0, sizeof(glm::vec3));

	glVertexArrayAttribFormat(mainVAO, 3, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(mainVAO, 3, tangentsVBO.id(), 0, sizeof(glm::vec3));

	// associate vertex attributes to binding indices.
	//										vertex attribute	binding index
	glVertexArrayAttribBinding(mainVAO,		0,					0					);
	glVertexArrayAttribBinding(mainVAO,		1,					1					);
	glVertexArrayAttribBinding(mainVAO,		2,					2					);
	glVertexArrayAttribBinding(mainVAO,		3,					3					);

	// bind skeletal VBO to binding index 4
	glVertexArrayVertexBuffer (mainVAO, 4, skeletalVBO.id(), 0, sizeof(VertexWeight));

	// specify skeletal vertex attribute
	glVertexArrayAttribIFormat(mainVAO, 4, 4, GL_INT, 0);
	glVertexArrayAttribFormat (mainVAO, 5, 4, GL_FLOAT, GL_FALSE, offsetof(VertexWeight, weights));

	// associate vertex attributes 5 & 6 to skeletal binding index 4. 
	glVertexArrayAttribBinding(mainVAO, 4, 4);
	glVertexArrayAttribBinding(mainVAO, 5, 4);

	// Enable attributes
	glEnableVertexArrayAttrib(mainVAO, 0);
	glEnableVertexArrayAttrib(mainVAO, 1);
	glEnableVertexArrayAttrib(mainVAO, 2);
	glEnableVertexArrayAttrib(mainVAO, 3);
	glEnableVertexArrayAttrib(mainVAO, 4);
	glEnableVertexArrayAttrib(mainVAO, 5);

	// ======================================================
	// Text VAO configuration
	// - No EBO. A much simpler VAO containing only position and texture.
	// ======================================================
	glGenVertexArrays(1, &textVAO);
	glBindVertexArray(textVAO);

	glBindBuffer(GL_ARRAY_BUFFER, textVBO.id());

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ======================================================
	// Particle VAO configuration
	// ======================================================
	glCreateVertexArrays(1, &particleVAO);

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(particleVAO, EBO.id());

	initialiseSSAO();
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &mainVAO);
	glDeleteVertexArrays(1, &textVAO);
	glDeleteVertexArrays(1, &particleVAO);

	if (ssaoNoiseTextureId != INVALID_ID) glDeleteTextures(1, &ssaoNoiseTextureId);
}

GLuint Renderer::getObjectId(glm::vec2 normalisedPosition) const {
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

GLuint Renderer::getObjectUiId(glm::vec2 normalisedPosition) const {
	GLuint objectId;

	int xOffset = static_cast<int>(normalisedPosition.x * uiObjectIdFrameBuffer.getWidth());	// x offset
	int yOffset = static_cast<int>(normalisedPosition.y * uiObjectIdFrameBuffer.getHeight());	// y offset

	glGetTextureSubImage(
		uiObjectIdFrameBuffer.textureIds()[0],
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

void Renderer::update([[maybe_unused]] float dt) {
	timeElapsed += dt;
}

void Renderer::renderMain(RenderConfig renderConfig) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	prepareRendering();

	// The renderer 
	switch (renderConfig)
	{
	// ===============================================
	// In this case, we focus on rendering to the editor's FBO.
	// ===============================================
	case RenderConfig::Editor:
		// Main render function
		if (isEditorScreenShown) {
			render(editorMainFrameBuffer, editorCamera, editorLights, editorClusterSSBO);

			// Apply HDR tone mapping + gamma correction post-processing
			renderHDRTonemapping(editorMainFrameBuffer);
			
			debugShader.setMatrix("model", glm::mat4{ 1.f });

			// render debug information..
			debugRender();

			// after debug rendering.. bind main position VBO back to VAO..
			glVertexArrayVertexBuffer(mainVAO, 0, positionsVBO.id(), 0, sizeof(glm::vec3));

			renderObjectIds();
		}
		
		// Main render function
		if (isGameScreenShown)
			render(gameMainFrameBuffer, gameCamera, gameLights, gameClusterSSBO);

		if (isGameScreenShown || isUIScreenShown)
			renderUI();
		
		if (isGameScreenShown) {
			overlayUIToBuffer(gameMainFrameBuffer);
			renderUiObjectIds();
		}

		// Apply HDR tone mapping + gamma correction post-processing
		renderHDRTonemapping(gameMainFrameBuffer);

		break;
	// ===============================================
	// In this case, we focus on rendering to the game's FBO.
	// ===============================================
	case RenderConfig::Game:
		// Main render function
		render(gameMainFrameBuffer, gameCamera, gameLights, gameClusterSSBO);
		renderUI();
		overlayUIToBuffer(gameMainFrameBuffer);

		// Apply HDR tone mapping + gamma correction post-processing
		renderHDRTonemapping(gameMainFrameBuffer);

		// only render to default FBO if it's truly game mode.
		if (renderConfig == RenderConfig::Game) {
			renderToDefaultFBO();
		}

		break;
	default:
		assert(false && "Forget to account for a case.");
		break;
	}
	// Bind back to default FBO for ImGui or Nova-Game to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderUI()
{
	glBindFramebuffer(GL_FRAMEBUFFER, uiMainFrameBuffer.fboId());
	glViewport(0, 0, uiMainFrameBuffer.getWidth(), uiMainFrameBuffer.getHeight());

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setDepthMode(DepthTestingMethod::NoDepthWriteTest);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);

	glBindVertexArray(textVAO);

	// UI draw calls here
	for (auto const& [layerName, entities] : engine.ecs.sceneManager.layers) {
		for (auto const& entity : entities) {
			Transform& transform = registry.get<Transform>(entity);
			EntityData& entityData = registry.get<EntityData>(entity);

			Image* image = registry.try_get<Image>(entity);
			Text* text = registry.try_get<Text>(entity);
			Button* button = registry.try_get<Button>(entity);

			if (!entityData.isActive) {
				continue;
			}

			if (image && engine.ecs.isComponentActive<Image>(entity)) {
				renderImage(transform, *image, button ? button->finalColor : whiteColor);
			}

			if (text && engine.ecs.isComponentActive<Text>(entity)) {
				renderText(transform, *text);
			}
		}
	}

	glBindVertexArray(mainVAO);
}

void Renderer::render(PairFrameBuffer& frameBuffers, Camera const& camera, LightSSBO& lightSSBO, BufferObject const& clusterSSBO) {
	// We clear this pair frame buffer..
	frameBuffers.clearFrameBuffers();

	// We upload camera data to the UBO..
	glNamedBufferSubData(sharedUBO.id(), 0, sizeof(glm::mat4x4), glm::value_ptr(camera.view()));
	glNamedBufferSubData(sharedUBO.id(), sizeof(glm::mat4x4), sizeof(glm::mat4x4), glm::value_ptr(camera.projection()));
	glNamedBufferSubData(sharedUBO.id(), 2 * sizeof(glm::mat4x4), sizeof(glm::mat4x4), glm::value_ptr(camera.projection() * camera.view()));

	// We perform frustum culling for models and lights..
	frustumCulling(camera);

	// We prepare our lights for rendering..
	// Build clusters and assign lights for clustered forward rendering
	// And render shadow maps..
	prepareLights(camera, lightSSBO);

	// Prepare cluster forwarded rendering..
	clusterBuilding(camera, clusterSSBO);

	// Generate shadow map based on camera position..
	shadowPass(camera);

	// We bind to the active framebuffer for majority of the in game rendering..
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	setBlendMode(BlendingConfig::Disabled);

	// We perform a depth pre pass..
	static constexpr GLenum buffers[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(frameBuffers.getActiveFrameBuffer().fboId(), 2, buffers);

	depthPrePass(camera);
	frameBuffers.getActiveFrameBuffer().setColorAttachmentActive(1);	// we restore back to default, writing to the 1st color attachment

	// We generate SSAO texture for forward rendering later..
	generateSSAO(frameBuffers, camera);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	// W Skybox	.
	renderSkyBox();

	// Because we had a depth pre pass, we can change depth function to equal.
	glDepthFunc(GL_EQUAL);

	// We render individual game objects..
	renderModels(camera);
	renderSkinnedModels(camera);

	// Restore default depth testing.
	glDepthFunc(GL_LESS);

	// Render particles
	renderParticles();

	// ======= Post Processing =======
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	setBlendMode(BlendingConfig::Disabled);

	// Apply bloom post processing via multi down and up samples.
	renderBloom(frameBuffers);

	// @TODO : Custom post processing stack.
	if(toPostProcess)
		renderPostProcessing(frameBuffers);
}

void Renderer::renderToDefaultFBO() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(
		engine.window.getGameViewPort().topLeftX,
		engine.window.getGameViewPort().topLeftY,
		engine.window.getGameViewPort().gameWidth,
		engine.window.getGameViewPort().gameHeight
	);

	overlayShader.use();
	overlayShader.setImageUniform("overlay", 0);
	glBindTextureUnit(0, getGameFrameBufferTexture());

	// VBO-less draw.
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glViewport(0, 0, gameWidth, gameHeight);

	glDisable(GL_BLEND);
}

void Renderer::renderBloom(PairFrameBuffer& frameBuffers) {
	// =======================================================
	// 1. We need to progressively down sample our HDR framebuffer.
	// =======================================================
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFrameBuffer.fboId());

	auto&& mipChain = bloomFrameBuffer.getMipChain();

	bloomDownSampleShader.use();
	bloomDownSampleShader.setVec2("srcResolution", gameSize);

	// We bind with the original scene..
	bloomDownSampleShader.setImageUniform("srcTexture", 0);
	glBindTextureUnit(0, frameBuffers.getActiveFrameBuffer().textureIds()[0]);

	// Progressively down sample through the mip chain
	for (auto&& mip : mipChain) {
		// update viewport to the mip map's size..
		glViewport(0, 0, mip.isize.x, mip.isize.y);

		// we bind this mipmap as our bloom framebuffer's new color attachment. (we do this every loop)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.id, 0);

		// Render screen-filled quad of resolution of current mip
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Set current mip resolution as srcResolution for next iteration
		bloomDownSampleShader.setVec2("srcResolution", mip.size);

		// Set current mip as texture input for next iteration
		glBindTextureUnit(0, mip.id);
	}

	// =======================================================
	// 2. We do the reverse, progressively upsample our downsampled scene with blur filter.
	// =======================================================
	bloomUpSampleShader.use();
	bloomUpSampleShader.setFloat("filterRadius", bloomFilterRadius);
	bloomUpSampleShader.setImageUniform("srcTexture", 0);

	// Enable additive blending
	// setBlendMode(CustomShader::BlendingConfig::AdditiveBlending);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (std::size_t i = mipChain.size() - 1; i > 0; i--) {
		auto&& mip		= mipChain[i];
		auto&& nextMip	= mipChain[i - 1];

		// Read from the smaller mip map size..
		glBindTextureUnit(0, mip.id);

		// Set framebuffer render target (we write to this texture)
		glViewport(0, 0, nextMip.isize.x, nextMip.isize.y);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.id, 0);

		// Render screen-filled quad of resolution of current mip
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Disable additive blending
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
	glDisable(GL_BLEND);

	// =======================================================
	// 3. We composite the original scene with this blurred, down and then upsampled bloom buffer.
	// =======================================================
	frameBuffers.swapFrameBuffer();

	glViewport(0, 0, gameWidth, gameHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	bloomFinalShader.use();
	glBindTextureUnit(0, frameBuffers.getReadFrameBuffer().textureIds()[0]);	// original scene
	glBindTextureUnit(1, bloomFrameBuffer.getMipChain()[0].id);					// blurred bright

	bloomFinalShader.setImageUniform("scene", 0);
	bloomFinalShader.setImageUniform("bloomBlur", 1);
	bloomFinalShader.setFloat("compositePercentage", bloomCompositePercentage);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::overlayUIToBuffer(PairFrameBuffer& target) {
	glBindFramebuffer(GL_FRAMEBUFFER, target.getActiveFrameBuffer().fboId());
	
	setBlendMode(BlendingConfig::AlphaBlending);

	overlayShader.use();
	overlayShader.setImageUniform("overlay", 0);
	glBindTextureUnit(0, getUIFrameBufferTexture());
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);
}

void Renderer::shadowPass([[maybe_unused]] Camera const& camera) {
	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	hasDirectionalLightShadowCaster = false;

	// let's find our directional light shadow caster..
	for (auto&& [entity, transform, entityData, light] : registry.view<Transform, EntityData, Light>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Light>(entity)) {
			continue;
		}

		// light not in camera frustum
		if (!transform.inCameraFrustum) {
			continue;
		}

		// not a shadow caster..
		if (!light.shadowCaster) {
			continue;
		}

		// We calculate the respective light matrix to generate our shadow maps..
		switch (light.type)
		{
		case Light::Type::Directional: {
			if (hasDirectionalLightShadowCaster) {
				Logger::warn("We support only 1 directional light shadow caster.");
				continue;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, directionalLightShadowFBO.fboId());
			glClear(GL_DEPTH_BUFFER_BIT);

			hasDirectionalLightShadowCaster = true;
			directionalLightShadowPass(transform.position, transform.front, light);

			break;
		}

		case Light::Type::PointLight:
			break;

		case Light::Type::Spotlight:
			break;
		}
	}

	glViewport(0, 0, gameWidth, gameHeight);
}

void Renderer::depthPrePass(Camera const& camera) {
	glEnable(GL_CULL_FACE);

	depthGBufferShader.use();
	depthGBufferShader.setMatrix("lightSpaceMatrix", camera.projection() * camera.view());

	// ===========================================================
	// 1. Render mesh renderer shadows
	// ===========================================================

	// indicate that this is a skinned mesh renderer..
	static const unsigned int isNotASkinnedMeshRenderer = 0;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isNotASkinnedMeshRenderer);

	for (auto const& [layerName, entities] : engine.ecs.sceneManager.layers) {
		for (auto const& entity : entities) {
			MeshRenderer* meshRenderer = registry.try_get<MeshRenderer>(entity);
			Transform const& transform = registry.get<Transform>(entity);
			EntityData const& entityData = registry.get<EntityData>(entity);

			if (!meshRenderer) {
				continue;
			}

			if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entity)) {
				continue;
			}

			if (!transform.inCameraFrustum) {
				continue;
			}

			// Retrieves model asset from asset manager.
			auto [model, _] = resourceManager.getResource<Model>(meshRenderer->modelId);

			if (!model) {
				// missing model.
				continue;
			}

			depthGBufferShader.setMatrix("model", transform.modelMatrix);
			depthGBufferShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));
			depthGBufferShader.setMatrix("normalMatrix", transform.normalMatrix);

			// Draw every mesh of a given model.
			for (auto const& mesh : model->meshes) {
				Material const* material = obtainMaterial(*meshRenderer, mesh);

				if (!material) {
					continue;
				}

				setupNormalMapUniforms(depthGBufferShader, *material);

				positionsVBO.uploadData(mesh.positions);
				textureCoordinatesVBO.uploadData(mesh.textureCoordinates);
				normalsVBO.uploadData(mesh.normals);
				tangentsVBO.uploadData(mesh.tangents);
				EBO.uploadData(mesh.indices);
				glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
			}
		}
	}

	// ===========================================================
	// 2. Render skinned mesh renderer shadows
	// ===========================================================
	// indicate that this is NOT a skinned mesh renderer..

	static const unsigned int isASkinnedMeshRenderer = 1;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isASkinnedMeshRenderer);

	for (auto const& [layerName, entities] : engine.ecs.sceneManager.layers) {
		for (auto const& entity : entities) {
			SkinnedMeshRenderer* skinnedMeshRenderer = registry.try_get<SkinnedMeshRenderer>(entity);
			Transform const& transform = registry.get<Transform>(entity);
			EntityData const& entityData = registry.get<EntityData>(entity);

			if (!skinnedMeshRenderer) {
				continue;
			}

			if (!entityData.isActive || !engine.ecs.isComponentActive<SkinnedMeshRenderer>(entity)) {
				continue;
			}

			if (!transform.inCameraFrustum) {
				continue;
			}

			// Retrieves model asset from asset manager.
			auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer->modelId);

			if (!model) {
				// missing model.
				continue;
			}

			depthGBufferShader.setMatrix("model", transform.modelMatrix);
			depthGBufferShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));

			// upload all bone matrices..
			glNamedBufferSubData(bonesSSBO.id(), sizeof(glm::vec4), skinnedMeshRenderer->bonesFinalMatrices.size() * sizeof(glm::mat4x4), skinnedMeshRenderer->bonesFinalMatrices.data());

			// Draw every mesh of a given model.
			for (auto const& mesh : model->meshes) {
				Material const* material = obtainMaterial(*skinnedMeshRenderer, mesh);

				if (!material) {
					continue;
				}

				setupNormalMapUniforms(depthGBufferShader, *material);

				positionsVBO.uploadData(mesh.positions);
				textureCoordinatesVBO.uploadData(mesh.textureCoordinates);
				normalsVBO.uploadData(mesh.normals);
				tangentsVBO.uploadData(mesh.tangents);
				EBO.uploadData(mesh.indices);
				skeletalVBO.uploadData(mesh.vertexWeights);
				glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
			}
		}
	}

	glDisable(GL_CULL_FACE);
}

void Renderer::generateSSAO(PairFrameBuffer& frameBuffers, Camera const& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer.fboId());
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	ssaoShader.use();
	ssaoShader.setVec2("screenDimensions", { gameWidth, gameHeight });
	ssaoShader.setFloat("near", camera.getNearPlaneDistance());
	ssaoShader.setFloat("far", camera.getFarPlaneDistance());

	// Bind depth, normal and noise texture..
	glBindTextureUnit(0, frameBuffers.getActiveFrameBuffer().depthStencilId());
	ssaoShader.setImageUniform("depthMap", 0);

	glBindTextureUnit(1, frameBuffers.getActiveFrameBuffer().textureIds()[1]);
	ssaoShader.setImageUniform("normalMap", 1);

	glBindTextureUnit(2, ssaoNoiseTextureId);
	ssaoShader.setImageUniform("noiseTexture", 2);

	// Render fullscreen quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::initialiseSSAO() {
	// https://learnopengl.com/Advanced-Lighting/SSAO
	std::vector<glm::vec4> ssaoKernel;
	constexpr int numOfKernalSamples = 64;

	// =======================================================
	// 1. We initialise a sample kernel to be used for SSAO.
	// =======================================================

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;

	for (unsigned int i = 0; i < numOfKernalSamples; ++i) {
		glm::vec4 sample (
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator),
			0
		);

		sample = glm::normalize(sample);	// Randomised direction, unit vector.
		sample *= randomFloats(generator);	// Randomised magnitude

		float scale = (float)i / numOfKernalSamples;	// Scale of vector, resulting in samples near the hemisphere.
		scale = std::lerp(0.1f, 1.0f, scale * scale);

		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// upload to UBO..
	glNamedBufferSubData(sharedUBO.id(), 3 * sizeof(glm::mat4x4), numOfKernalSamples * sizeof(glm::vec4), ssaoKernel.data());

	// =======================================================
	// 2. Generating a 4x4 random noise
	// =======================================================
	// 2.1 Value generation.

	std::vector<glm::vec3> ssaoNoise;

	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f
		);

		ssaoNoise.push_back(noise);
	}

	// 2.2 Texture creation.
	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoNoiseTextureId);
	glTextureStorage2D(ssaoNoiseTextureId, 1, GL_RGB8, 4, 4);
	glTextureParameteri(ssaoNoiseTextureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoNoiseTextureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoNoiseTextureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(ssaoNoiseTextureId, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureSubImage2D(ssaoNoiseTextureId, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
}

void Renderer::submitSelectedObjects(std::vector<entt::entity> const& entities) {
	selectedEntities = entities;
}

GLuint Renderer::getEditorFrameBufferTexture() const {
	return editorMainFrameBuffer.getActiveFrameBuffer().textureIds()[0];
}

GLuint Renderer::getGameFrameBufferTexture() const {
	return gameMainFrameBuffer.getActiveFrameBuffer().textureIds()[0];
}

GLuint Renderer::getUIFrameBufferTexture() const
{
	return uiMainFrameBuffer.textureIds()[0];
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

Camera& Renderer::getEditorCamera() {
	return editorCamera;
}

Camera const& Renderer::getEditorCamera() const {
	return editorCamera;
}

Camera& Renderer::getGameCamera() {
	return gameCamera;
}

Camera const& Renderer::getGameCamera() const {
	return gameCamera;
}

void Renderer::recompileShaders() {
	skyboxShader.recompile();
	textShader.recompile();
	toneMappingShader.recompile();
	particleShader.recompile();
	texture2dShader.recompile();
	bloomDownSampleShader.recompile();
	bloomUpSampleShader.recompile();
	bloomFinalShader.recompile();
	postprocessingShader.recompile();

	clusterBuildingCompute.recompile();
	clusterLightCompute.recompile();

	ssaoShader.recompile();

	auto [defaultPBRShader, _] = resourceManager.getResource<CustomShader>(DEFAULT_PBR_SHADER_ID);

	if (defaultPBRShader) {
		defaultPBRShader->compile();
	}
}

void Renderer::debugRenderPhysicsCollider() {
	// Bind Debug Physics VBO to VAO.
	constexpr GLuint positionBindingIndex = 0;
	glVertexArrayVertexBuffer(mainVAO, positionBindingIndex, debugPhysicsVBO.id(), 0, sizeof(glm::vec3));

	// ================================================
	// 1. We first render all debug shapes (triangles and lines) into a separate FBO
	// ================================================
	glBindVertexArray(mainVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, physicsDebugFrameBuffer.fboId());

	// Clear physics debug framebuffer.
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
	debugShader.use();
	debugShader.setVec4("color", { 0.f, 1.f, 0.f, 0.2f });
	glDrawArrays(GL_TRIANGLES, 0, numOfPhysicsDebugTriangles * 3);

	// enable wireframe mode only for debug overlay.
	debugShader.setVec4("color", { 0.f, 1.f, 0.f, 1.f });
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, numOfPhysicsDebugTriangles * 3);

	// disable wireframe mode, restoring to normal fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glVertexArrayVertexBuffer(mainVAO, positionBindingIndex, debugPhysicsLineVBO.id(), 0, sizeof(glm::vec3));

	glDisable(GL_CULL_FACE);

	debugShader.setVec4("color", { 1.f, 0.2f, 0.2f, 1.f });
	glDrawArrays(GL_LINES, 0, numOfPhysicsDebugLines * 2);

	numOfPhysicsDebugTriangles = 0;
	numOfPhysicsDebugLines = 0;

	glDisable(GL_DEPTH_TEST);
	overlayShader.use();
	
	// ================================================
	// 2. We overlay this resulting debug shapes into our main FBO, with alpha blending.
	// (so post processing)
	// ================================================

	setBlendMode(BlendingConfig::AlphaBlending);
	glBindFramebuffer(GL_FRAMEBUFFER, editorMainFrameBuffer.getActiveFrameBuffer().fboId());
	
	// set image uniform accordingly..
	glBindTextureUnit(0, physicsDebugFrameBuffer.textureIds()[0]);
	overlayShader.setImageUniform("overlay", 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);
}


void Renderer::debugRenderNavMesh() {
	// Bind Debug Navmesh VBO to VAO.
	constexpr GLuint positionBindingIndex = 0;
	glVertexArrayVertexBuffer(mainVAO, positionBindingIndex, debugNavMeshVBO.id(), 0, sizeof(glm::vec3));

	glBindVertexArray(mainVAO);
	
	// glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());

	glDisable(GL_STENCIL_TEST);
	setBlendMode(BlendingConfig::AlphaBlending);
	glEnable(GL_DEPTH_TEST);

	debugShader.use();
	debugShader.setVec4("color", { 0.f, 0.8f, 0.8f, 0.5f });
	glDrawArrays(GL_TRIANGLES, 0, numOfNavMeshDebugTriangles * 3);

	setBlendMode(BlendingConfig::Disabled);
	numOfNavMeshDebugTriangles = 0;
}

void Renderer::debugRenderParticleEmissionShape()
{
	debugShader.use();
	debugShader.setVec4("color", { 0.f,1.0f,1.0f,1.0f });
	glVertexArrayVertexBuffer(mainVAO, 0, debugParticleShapeVBO.id(), 0, sizeof(glm::vec3));

	glBindVertexArray(mainVAO);

	glEnable(GL_DEPTH_TEST);
	setBlendMode(BlendingConfig::AlphaBlending);

	for (auto&& [entity, transform, emitter] : registry.view<Transform, ParticleEmitter>().each()) {
		glm::mat4 model = glm::identity<glm::mat4>();
		model = glm::translate(model, transform.position);
		model = model * glm::mat4_cast(transform.rotation);
		debugShader.setMatrix("model", model);
		switch (emitter.particleEmissionTypeSelection.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXY(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisYZ(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			debugParticleShapeVBO.uploadData(DebugShapes::Cube(emitter.particleEmissionTypeSelection.cubeEmitter.min, emitter.particleEmissionTypeSelection.cubeEmitter.max));
			glDrawArrays(GL_LINES, 0, 24);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
			debugParticleShapeVBO.uploadData(DebugShapes::Edge(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINES, 0, 2);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			debugParticleShapeVBO.uploadData(DebugShapes::HemisphereAxisXY(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS / 2 + 1);
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			debugParticleShapeVBO.uploadData(DebugShapes::HemisphereAxisYZ(emitter.particleEmissionTypeSelection.radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS / 2 + 1);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
			ConeEmitter& coneEmitter{ emitter.particleEmissionTypeSelection.coneEmitter };
			RadiusEmitter& radiusEmitter{ emitter.particleEmissionTypeSelection.radiusEmitter };
			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(radiusEmitter.radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			debugParticleShapeVBO.uploadData(DebugShapes::ConeEdges(radiusEmitter.radius, coneEmitter.arc, coneEmitter.distance));
			glDrawArrays(GL_LINES, 0, 8);
			debugParticleShapeVBO.uploadData(DebugShapes::ConeOuterAxisXZ(radiusEmitter.radius,coneEmitter.arc,coneEmitter.distance));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
			break;
		}
	}
}

void Renderer::debugRenderBoundingVolume() {
	debugShader.use();
	debugShader.setVec4("color", { 1.f, 1.0f, 0.0f, 1.0f });
	debugShader.setMatrix("model", glm::identity<glm::mat4>());

	glVertexArrayVertexBuffer(mainVAO, 0, debugParticleShapeVBO.id(), 0, sizeof(glm::vec3));
	glBindVertexArray(mainVAO);
	glEnable(GL_DEPTH_TEST);

	for (auto&& [entityID, entityData, transform, meshRenderer] : engine.ecs.registry.view<EntityData, Transform, MeshRenderer>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entityID)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = engine.resourceManager.getResource<Model>(meshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		debugParticleShapeVBO.uploadData(DebugShapes::Cube(transform.boundingBox));
		glDrawArrays(GL_LINES, 0, 24);
	}

	for (auto&& [entityID, transform, cameraComponent] : engine.ecs.registry.view<Transform, CameraComponent>().each()) {
		if (!cameraComponent.camStatus) {
			continue;
		}
		
		// We found our game camera..
		debugShader.setVec4("color", { 0.f, 1.0f, 1.0f, 1.0f });

		auto vertices = DebugShapes::CameraFrustumOutline(transform.position, gameCamera);
		debugParticleShapeVBO.uploadData(vertices);
		glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
		
		setBlendMode(BlendingConfig::AlphaBlending);
		debugShader.setVec4("color", { 0.f, 1.0f, 1.0f, 0.2f });
		
		vertices = DebugShapes::CameraFrustum(transform.position, gameCamera);
		debugParticleShapeVBO.uploadData(vertices);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
		
		break;
	}
}

void Renderer::debugRenderClusters() {
	glBindVertexArray(mainVAO);
	glVertexArrayVertexBuffer(mainVAO, 0, debugParticleShapeVBO.id(), 0, sizeof(glm::vec3));

	static std::array<Cluster, numClusters> clusters {};

	// retrieve all cluster information..
	glGetNamedBufferSubData(gameClusterSSBO.id(), 0, numClusters * sizeof(Cluster), clusters.data());

	std::size_t offset = 0;

	// upload cluster AABB to VBO.
	for (auto const& cluster : clusters) {
		glm::vec3 center = glm::vec4{ (cluster.maxPoint + cluster.minPoint) / 2.f };
		glm::vec3 extents = glm::vec3{ cluster.maxPoint } - center;

		std::vector<glm::vec3> cubeVertices = DebugShapes::Cube({ center, extents });
		std::size_t size = cubeVertices.size();

		debugParticleShapeVBO.uploadData(std::move(cubeVertices), offset);
		offset += size * sizeof(glm::vec3);
	}

	debugShader.use();

	// because clusters are defined in view space, the inverse of the camera's view matrix brings clusters to world space. 
	debugShader.setMatrix("model", glm::inverse(gameCamera.view()));	
	glDrawArrays(GL_LINES, 0, 24 * numClusters);
}

void Renderer::submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3) {
	static std::vector<glm::vec3> vertices(3);

	if (numOfPhysicsDebugTriangles > MAX_DEBUG_TRIANGLES) {
		std::cerr << "too much triangles!\n";
		return;
	}

	vertices[0] = vertice1;
	vertices[1] = vertice2;
	vertices[2] = vertice3;

	debugPhysicsVBO.uploadData(vertices, 3 * numOfPhysicsDebugTriangles * sizeof(glm::vec3));
	++numOfPhysicsDebugTriangles;
}

void Renderer::submitLine(glm::vec3 vertice1, glm::vec3 vertice2) {
	if (numOfPhysicsDebugLines > MAX_DEBUG_LINES) {
		std::cerr << "too much triangles!\n";
		return;
	}

	debugPhysicsLineVBO.uploadData(std::vector<glm::vec3>{ { vertice1 }, { vertice2 } }, 2 * numOfPhysicsDebugLines * sizeof(glm::vec3));
	++numOfPhysicsDebugLines;
}

void Renderer::submitNavMeshTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3){
	if (numOfNavMeshDebugTriangles > MAX_DEBUG_TRIANGLES) {
		std::cerr << "too much triangles!\n";
		return;
	}

	debugNavMeshVBO.uploadData(std::vector<glm::vec3>{ { vertice1 }, { vertice2 }, { vertice3 } }, 3 * numOfNavMeshDebugTriangles * sizeof(glm::vec3));
	++numOfNavMeshDebugTriangles;
}

void Renderer::prepareRendering() {
#if defined(DEBUG)
	ZoneScopedC(tracy::Color::PaleVioletRed1);
#endif

	// =================================================================
	// Configure pre rendering settings
	// =================================================================
	glEnable(GL_DITHER);
	setDepthMode(DepthTestingMethod::DepthTest);

	// bind the VBOs to their respective binding index
	glVertexArrayVertexBuffer(mainVAO, 0, positionsVBO.id(),			0, sizeof(glm::vec3));
	glVertexArrayVertexBuffer(mainVAO, 1, textureCoordinatesVBO.id(),	0, sizeof(glm::vec2));
	glVertexArrayVertexBuffer(mainVAO, 2, normalsVBO.id(),				0, sizeof(glm::vec3));
	glVertexArrayVertexBuffer(mainVAO, 3, tangentsVBO.id(),				0, sizeof(glm::vec3));
	glBindVertexArray(mainVAO);

	// =================================================================
	// Clear frame buffers.
	// =================================================================
	
	// Clear default framebuffer.
	// glBindFramebuffer(GL_FRAMEBUFFER, 0); // we bind to default FBO at the end of our render.
	glClearColor(0.05f, 0.05f, 0.05f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	engine.particleSystem.populateParticleLights(MAX_NUMBER_OF_LIGHT);
}

void Renderer::renderSkyBox() {
#if defined(DEBUG)
	ZoneScopedC(tracy::Color::PaleVioletRed1);
#endif
	glDisable(GL_DEPTH_TEST);

	for (auto&& [entityId,entityData, skyBox] : registry.view<EntityData,SkyBox>().each()) {
		auto [asset, status] = resourceManager.getResource<CubeMap>(skyBox.cubeMapId);

		// skybox not loaded..
		if (!asset || !entityData.isActive || !engine.ecs.isComponentActive<SkyBox>(entityId)) {
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

void Renderer::renderModels(Camera const& camera) {
#if defined(DEBUG)
	ZoneScopedC(tracy::Color::PaleVioletRed1);
#endif

	glEnable(GL_CULL_FACE);

	// indicate that this is NOT a skinned mesh renderer..
	static const unsigned int isNotASkinnedMeshRenderer = 0;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isNotASkinnedMeshRenderer);

	for (auto const& [layerName, entities] : engine.ecs.sceneManager.layers) {
		for (auto const& entity : entities) {
			MeshRenderer* meshRenderer = registry.try_get<MeshRenderer>(entity);
			Transform const& transform = registry.get<Transform>(entity);
			EntityData const& entityData = registry.get<EntityData>(entity);

			if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entity)) {
				continue;
			}

			// frustum culling :)
			if (!transform.inCameraFrustum) {
				continue;
			}

			if (!meshRenderer) {
				continue;
			}

			// Retrieves model asset from asset manager.
			auto [model, _] = resourceManager.getResource<Model>(meshRenderer->modelId);

			if (!model) {
				// missing model.
				continue;
			}

			// If a model has only one submesh, we render all materials attached to this mesh renderer..
			if (model->meshes.size() == 1) {
				// Draw every material of a this mesh.
				auto const& mesh = model->meshes[0];

				for (auto const& materialId : meshRenderer->materialIds) {
					auto&& [material, __] = resourceManager.getResource<Material>(materialId);

					if (!material) {
						continue;
					}

					// Use the correct shader and configure it's required uniforms..
					CustomShader* shader = setupMaterial(camera, *material, transform, model->scale);

					if (shader) {
						// time to draw!
						renderMesh(mesh, shader->customShaderData.pipeline, MeshType::Normal);
					}
				}
			}
			else {
				// Draw every mesh of a given model.
				for (auto const& mesh : model->meshes) {
					Material const* material = obtainMaterial(*meshRenderer, mesh);

					// Use the correct shader and configure it's required uniforms..
					CustomShader* shader = setupMaterial(camera, *material, transform, model->scale);

					if (shader) {
						// time to draw!
						renderMesh(mesh, shader->customShaderData.pipeline, MeshType::Normal);
					}
				}
			}
		}
	}
		
	glDisable(GL_CULL_FACE);
}

void Renderer::renderText(Transform const& transform, Text const& text)
{
	struct Vertex {
		GLfloat x, y;   // screen position
		GLfloat s, t;   // texture coordinates
	};

	// activate corresponding render state	
	textShader.use();
	textShader.setMatrix("projection", UIProjection);

	glActiveTexture(GL_TEXTURE0);

	// Retrieves font asset from asset manager.
	auto [font, _] = resourceManager.getResource<Font>(text.font);

	if (!font) {
		return;
	}

	if (text.text.empty()) {
		return;
	}

	textShader.setVec3("textColor", text.fontColor);

	Font::Atlas const& atlas = font->getAtlas();
	float fontScale = static_cast<float>(text.fontSize) / font->getFontSize();

	float x = transform.position.x;
	float y = transform.position.y;

	std::vector<Vertex> vertices;
	vertices.reserve(text.text.size() * 6); // 6 vertices per char, 4 floats per vertex

	std::string::const_iterator c;
	for (c = text.text.begin(); c != text.text.end(); c++)
	{
		const Font::Character& ch = font->getCharacters().at(*c);

		float xpos = x + ch.bearing.x * fontScale;
		float ypos = y - (ch.size.y - ch.bearing.y) * fontScale;

		float w = ch.size.x * fontScale;
		float h = ch.size.y * fontScale;

		// advance cursors for next glyph 
		x += ch.advance * fontScale;

		// Skip invisible glyphs
		if (w == 0 || h == 0)
			continue;

		float tx = ch.tx;                   // texture X offset in atlas
		float ty = 0.f;                     // texture Y offset in atlas
		float tw = static_cast<float>(ch.size.x) / atlas.width;
		float th = static_cast<float>(ch.size.y) / atlas.height;

		// 6 vertices per quad (2 triangles)
		vertices.push_back({ xpos,     ypos + h, tx,       ty });
		vertices.push_back({ xpos,     ypos,     tx,       ty + th });
		vertices.push_back({ xpos + w, ypos,     tx + tw,  ty + th });

		vertices.push_back({ xpos,     ypos + h, tx,       ty });
		vertices.push_back({ xpos + w, ypos,     tx + tw,  ty + th });
		vertices.push_back({ xpos + w, ypos + h, tx + tw,  ty });
	}

	// Bind font atlas texture once per string
	glBindTexture(GL_TEXTURE_2D, atlas.textureId);

	// Upload vertex data for all glyphs in this string
	textVBO.uploadData(vertices, 0);

	// Draw all characters in one batched call
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
}

void Renderer::renderImage(Transform const& transform, Image const& image, ColorA const& colorMultiplier)
{
	texture2dShader.use();

	// Get texture resource
	auto [textureAsset, status] = resourceManager.getResource<Texture>(image.texture);
	if (!textureAsset) {
		return;
	}

	GLuint textureId = textureAsset->getTextureId();
	if (textureId == 0) {
		return;
	}

	texture2dShader.setVec4("tintColor", image.colorTint * colorMultiplier);
	texture2dShader.setMatrix("model", transform.modelMatrix);
	texture2dShader.setInt("anchorMode", static_cast<int>(image.anchorMode));

	texture2dShader.setVec2("textureCoordinatesRange", image.textureCoordinatesRange);
	texture2dShader.setMatrix("uiProjection", UIProjection);
	texture2dShader.setInt("image", 0);
	texture2dShader.setBool("toFlip", image.toFlip);
	texture2dShader.setBool("toTile", image.toTile);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Renderer::renderSkinnedModels(Camera const& camera) {
#if defined(DEBUG)
	ZoneScopedC(tracy::Color::PaleVioletRed1);
#endif

	// enable back face culling for our 3d models..
	glEnable(GL_CULL_FACE);

	// indicate that this is a skinned mesh renderer..
	static const unsigned int isSkinnedMeshRenderer = 1;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isSkinnedMeshRenderer);

	for (auto&& [entity, transform, entityData, skinnedMeshRenderer] : registry.view<Transform, EntityData, SkinnedMeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<SkinnedMeshRenderer>(entity)) {
			continue;
		}
		
		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}
		
		// frustum culling :)
		if (!transform.inCameraFrustum) {
			continue;
		}

		// upload all bone matrices..
		glNamedBufferSubData(bonesSSBO.id(), sizeof(glm::vec4), skinnedMeshRenderer.bonesFinalMatrices.size() * sizeof(glm::mat4x4), skinnedMeshRenderer.bonesFinalMatrices.data());

		// If a model has only one submesh, we render all materials attached to this mesh renderer..
		if (model->meshes.size() == 1) {
			// Draw every material of a this mesh.
			auto const& mesh = model->meshes[0];

			for (auto const& materialId : skinnedMeshRenderer.materialIds) {
				auto&& [material, __] = resourceManager.getResource<Material>(materialId);

				if (!material) {
					continue;
				}

				// Use the correct shader and configure it's required uniforms..
				CustomShader* shader = setupMaterial(camera, *material, transform, model->scale);

				if (shader) {
					// time to draw!
					renderMesh(mesh, shader->customShaderData.pipeline, MeshType::Skinned);
				}
			}
		}
		else {
			for (auto const& mesh : model->meshes) {
				Material const* material = obtainMaterial(skinnedMeshRenderer, mesh);

				if (!material) {
					continue;
				}

				// Use the correct shader and configure it's required uniforms..
				CustomShader* shader = setupMaterial(camera, *material, transform, model->scale);

				if (shader) {
					// time to draw!
					renderMesh(mesh, shader->customShaderData.pipeline, MeshType::Skinned);
				}
			}
		}
	}

	glDisable(GL_CULL_FACE);
}

void Renderer::renderOutline() {
#if 0
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
#endif
}

void Renderer::renderParticles()
{
	glBindVertexArray(particleVAO);
	setBlendMode(BlendingConfig::AlphaBlending);
	particleShader.use();
	particleShader.setUInt("maxParticlesPerTexture", engine.particleSystem.MAX_PARTICLES_PER_TEXTURE);
	
	// Disable writing to depth buffer for particles
	glDepthMask(GL_FALSE);
	std::vector<int> squareIndices{ 0, 2, 1, 2, 0, 3 };
	EBO.uploadData(squareIndices);

	// render texture by texture
	for (int textureIndex{}; textureIndex < engine.particleSystem.usedTextures.size(); ++textureIndex) {
		auto&& [texture, result] = resourceManager.getResource<Texture>(engine.particleSystem.usedTextures[textureIndex]);
		if (!texture)
			continue;
		glBindTextureUnit(0, texture->getTextureId());
		particleShader.setUInt("textureIndex", textureIndex);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, engine.particleSystem.MAX_PARTICLES_PER_TEXTURE);
	}

	// Renable Depth Writing for other rendering
	glDepthMask(GL_TRUE);
}

Material const* Renderer::obtainMaterial(MeshRenderer const& meshRenderer, Mesh const& mesh) {
	if (mesh.materialIndex >= meshRenderer.materialIds.size()) {
		return nullptr;
	}

	auto&& [material, _] = resourceManager.getResource<Material>(meshRenderer.materialIds[mesh.materialIndex]);
	return material;
}

Material const* Renderer::obtainMaterial(SkinnedMeshRenderer const& skinnedMeshRenderer, Mesh const& mesh) {
	if (mesh.materialIndex >= skinnedMeshRenderer.materialIds.size()) {
		return nullptr;
	}

	auto&& [material, _] = resourceManager.getResource<Material>(skinnedMeshRenderer.materialIds[mesh.materialIndex]);
	return material;
}

void Renderer::frustumCulling(Camera const& camera) {
	Frustum const& cameraFrustum = calculateCameraFrustum(camera);

	// ============================================
	// We do frustum culling check for mesh & skinned mesh renderer
	// ============================================
	auto calculateFrustumCulling = [&](Model* model, Transform& transform) {
		if (!model) {
			return;
		}

		// Calculate appropriate bounding box.
		transform.boundingBox = calculateAABB(*model, transform);
		transform.inCameraFrustum = cameraFrustum.isAABBInFrustum(transform.boundingBox);
	};

	for (auto&& [entityID, entityData, transform, meshRenderer] : registry.view<EntityData, Transform, MeshRenderer>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entityID)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = engine.resourceManager.getResource<Model>(meshRenderer.modelId);
		calculateFrustumCulling(model, transform);
	}

	for (auto&& [entityID, entityData, transform, skinnedMeshRenderer] : registry.view<EntityData, Transform, SkinnedMeshRenderer>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entityID)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = engine.resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);
		calculateFrustumCulling(model, transform);
	}

	// ============================================
	// We do frustum culling check for lights
	// ============================================
	for (auto&& [entityID, entityData, transform, light] : registry.view<EntityData, Transform, Light>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<Light>(entityID)) {
			continue;
		}

		if (light.type == Light::Type::Directional) {
			// directional lights affects everything :)
			transform.inCameraFrustum = true;
		}
		else {
			transform.inCameraFrustum = cameraFrustum.isSphereInFrustum(Sphere{ transform.position, light.radius });
		}
	}
}

void Renderer::setBlendMode(BlendingConfig configuration) {
	switch (configuration) {
		using enum BlendingConfig;
	case AlphaBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	default:
		assert(false && "Forget to handle other case.");
		break;
	}
}

void Renderer::setDepthMode(DepthTestingMethod configuration) {
	switch (configuration) {
		using enum DepthTestingMethod;
	case DepthTest:
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		break;
	case NoDepthWrite:
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		break;
	case NoDepthWriteTest:
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		break;
	default:
		assert(false && "Forget to handle other case.");
		break;
	}
}

void Renderer::setCullMode(CullingConfig configuration) {
	switch (configuration) {
		using enum CullingConfig;
	case Enable:
		glEnable(GL_CULL_FACE);
		break;
	case Disable:
		glDisable(GL_CULL_FACE);
		break;
	default:
		assert(false && "Forget to handle other case.");
		break;
	}
}

void Renderer::prepareLights([[maybe_unused]] Camera const& camera, LightSSBO& lightSSBO) {
	// we need to set up light data..
	std::array<PointLightData, MAX_NUMBER_OF_LIGHT>			pointLightData;
	std::array<DirectionalLightData, MAX_NUMBER_OF_LIGHT>	directionalLightData;
	std::array<SpotLightData, MAX_NUMBER_OF_LIGHT>			spotLightData;

	unsigned int numOfPtLights = 0;
	unsigned int numOfDirLights = 0;
	unsigned int numOfSpotLights = 0;

	for (auto&& [entity, transform, entityData, light] : registry.view<Transform, EntityData, Light>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Light>(entity)) {
			continue;
		}

		// No point in including light calculation for point light sources with influences outside the camera frustum.
		if (!transform.inCameraFrustum)
			continue;

		switch (light.type)
		{
		case Light::Type::PointLight:
			if (numOfPtLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Max number of point lights reached!");
				continue;
			}
			pointLightData[numOfPtLights++] = {
				transform.position,
				glm::vec3{ light.color } *light.intensity,
				light.attenuation,
				light.radius
			};
			break;

		case Light::Type::Directional:
		{
			if (numOfDirLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Max number of directional lights reached!");
				continue;
			}

			directionalLightData[numOfDirLights++] = {
				glm::normalize(transform.front),
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
				light.outerCutOffAngle,
				light.radius
			};
			break;
		}

		}
	}

	// prepare the light SSBOs. we bind light SSBO to binding point of 0, 1 & 2
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO.pointLight.id()); 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightSSBO.directionalLight.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightSSBO.spotLight.id());

	// Send it over to SSBO.
	glNamedBufferSubData(lightSSBO.pointLight.id(), 0, sizeof(unsigned int), &numOfPtLights);	// copy the unsigned int representing number of lights into SSBO.
	glNamedBufferSubData(lightSSBO.directionalLight.id(), 0, sizeof(unsigned int), &numOfDirLights);
	glNamedBufferSubData(lightSSBO.spotLight.id(), 0, sizeof(unsigned int), &numOfSpotLights);

	// copy all the light data to the SSBO.
	// offset is an alignment of LightData!! because of alignment requirements of this struct!
	// omdayz..
	glNamedBufferSubData(lightSSBO.pointLight.id(), alignof(PointLightData), numOfPtLights * sizeof(PointLightData), pointLightData.data());
	glNamedBufferSubData(lightSSBO.directionalLight.id(), alignof(DirectionalLightData), numOfDirLights * sizeof(DirectionalLightData), directionalLightData.data());
	glNamedBufferSubData(lightSSBO.spotLight.id(), alignof(SpotLightData), numOfSpotLights * sizeof(SpotLightData), spotLightData.data());
}

void Renderer::clusterBuilding(Camera const& camera, BufferObject const& clusterSSBO) {
#if 1
	// we bind bones SSBO to 7.
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, clusterSSBO.id());

	// ===============================================
	// 1. We first build the clusters AABB from screen space..
	// ===============================================
	clusterBuildingCompute.use();
	clusterBuildingCompute.setFloat("zNear", camera.getNearPlaneDistance());
	clusterBuildingCompute.setFloat("zFar", camera.getFarPlaneDistance());
	clusterBuildingCompute.setMatrix("inverseProjection", glm::inverse(camera.projection()));
	clusterBuildingCompute.setUVec3("gridSize", { gridSizeX, gridSizeY, gridSizeZ });
	clusterBuildingCompute.setUVec2("screenDimensions", { gameWidth, gameHeight });

	glDispatchCompute(gridSizeX, gridSizeY, gridSizeZ);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// ===============================================
	// 2. We assign lights to their respective clusters..
	// ===============================================
	clusterLightCompute.use();

	glDispatchCompute(27, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
#endif
}

Frustum Renderer::calculateCameraFrustum(Camera const& camera) {
	Frustum frustum;

	// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	// https://www.reddit.com/r/opengl/comments/1fstgtt/strange_issue_with_frustum_extraction/
	glm::mat4x4 m = camera.projection() * camera.view();

	frustum.leftPlane = { glm::row(m, 3) + glm::row(m, 0) };
	frustum.rightPlane = { glm::row(m, 3) - glm::row(m, 0) };
	frustum.bottomPlane = { glm::row(m, 3) + glm::row(m, 1) };
	frustum.topPlane = { glm::row(m, 3) - glm::row(m, 1) };
	frustum.nearPlane = { glm::row(m, 3) + glm::row(m, 2) };
	frustum.farPlane = { glm::row(m, 3) - glm::row(m, 2) };

	frustum.leftPlane.normalize();
	frustum.rightPlane.normalize();
	frustum.bottomPlane.normalize();
	frustum.topPlane.normalize();
	frustum.nearPlane.normalize();
	frustum.farPlane.normalize();

	return frustum;
}

AABB Renderer::calculateAABB(Model const& model, Transform const& transform) {
	if (transform.rotation == glm::quat_identity<float, glm::highp>()) {
		return {
			model.center * transform.scale + transform.position,
			model.extents * transform.scale
		};
	}

	// https://stackoverflow.com/questions/34619341/can-axis-aligned-bounding-boxes-be-recalculated-after-rotation-of-object-using-t
	// We need to transform our AABBs
	glm::vec3 pointOne = model.maxBound;
	glm::vec3 pointTwo = glm::vec3{ model.maxBound.x, model.maxBound.y, model.minBound.z };
	glm::vec3 pointThree = glm::vec3{ model.maxBound.x, model.minBound.y, model.minBound.z };
	glm::vec3 pointFour = glm::vec3{ model.maxBound.x, model.minBound.y, model.maxBound.z };
	glm::vec3 pointFive = glm::vec3{ model.minBound.x, model.minBound.y, model.maxBound.z };
	glm::vec3 pointSix = glm::vec3{ model.minBound.x, model.maxBound.y, model.maxBound.z };
	glm::vec3 pointSeven = glm::vec3{ model.minBound.x, model.maxBound.y, model.minBound.z };
	glm::vec3 pointEight = model.minBound;

	pointOne = transform.rotation * (transform.scale * pointOne);
	pointTwo = transform.rotation * (transform.scale * pointTwo);
	pointThree = transform.rotation * (transform.scale * pointThree);
	pointFour = transform.rotation * (transform.scale * pointFour);
	pointFive = transform.rotation * (transform.scale * pointFive);
	pointSix = transform.rotation * (transform.scale * pointSix);
	pointSeven = transform.rotation * (transform.scale * pointSeven);
	pointEight = transform.rotation * (transform.scale * pointEight);

	glm::vec3 newMaxBound = findMaxBound(pointOne, pointTwo, pointThree, pointFour, pointFive, pointSix, pointSeven, pointEight);
	glm::vec3 newMinBound = findMinBound(pointOne, pointTwo, pointThree, pointFour, pointFive, pointSix, pointSeven, pointEight);
	glm::vec3 newCenter = (newMaxBound + newMinBound) / 2.f;
	glm::vec3 newExtent = newMaxBound - newCenter;

	return {
		newCenter + transform.position,
		newExtent
	};
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

void Renderer::directionalLightShadowPass(glm::vec3 const& cameraPosition, glm::vec3 const& lightFront, Light const& light) {
	glm::mat4 lightProjection = glm::ortho(-light.orthogonalShadowCasterSize, light.orthogonalShadowCasterSize, -light.orthogonalShadowCasterSize, light.orthogonalShadowCasterSize, light.shadowNearPlane, light.shadowFarPlane);
	glm::mat4 lightView = glm::lookAt(cameraPosition, cameraPosition + lightFront, glm::vec3(0.0f, 1.0f, 0.0f));
	directionalLightViewMatrix = lightProjection * lightView;
	directionalLightDir = lightFront;

	glEnable(GL_CULL_FACE);

	shadowMapShader.use();
	shadowMapShader.setMatrix("lightSpaceMatrix", directionalLightViewMatrix);

	// ===========================================================
	// 1. Render mesh renderer shadows
	// ===========================================================

	// indicate that this is a skinned mesh renderer..
	static const unsigned int isNotASkinnedMeshRenderer = 0;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isNotASkinnedMeshRenderer);

	for (auto const& [entity, entityData, transform, meshRenderer] : registry.view<EntityData, Transform, MeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entity)) {
			continue;
		}

		// not a shadow caster..
		if (!meshRenderer.castShadow) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		shadowMapShader.setMatrix("model", transform.modelMatrix);
		shadowMapShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));

		meshRenderer.shadowCullFrontFace ? glCullFace(GL_FRONT) : glCullFace(GL_BACK);

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			positionsVBO.uploadData(mesh.positions);
			EBO.uploadData(mesh.indices);
			skeletalVBO.uploadData(mesh.vertexWeights);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
	
	// ===========================================================
	// 2. Render skinned mesh renderer shadows
	// ===========================================================
	// indicate that this is NOT a skinned mesh renderer..
	glCullFace(GL_FRONT);

	static const unsigned int isASkinnedMeshRenderer = 1;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isASkinnedMeshRenderer);

	for (auto const& [entity, entityData, transform, skinnedMeshRenderer] : registry.view<EntityData, Transform, SkinnedMeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<SkinnedMeshRenderer>(entity)) {
			continue;
		}

		// not a shadow caster..
		if (!skinnedMeshRenderer.castShadow) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		shadowMapShader.setMatrix("model", transform.modelMatrix);
		shadowMapShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));

		// upload all bone matrices..
		glNamedBufferSubData(bonesSSBO.id(), sizeof(glm::vec4), skinnedMeshRenderer.bonesFinalMatrices.size() * sizeof(glm::mat4x4), skinnedMeshRenderer.bonesFinalMatrices.data());

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			positionsVBO.uploadData(mesh.positions);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}

	glCullFace(GL_BACK);
}

void Renderer::setupNormalMapUniforms(Shader& shader, Material const& material) {
	// Setting up normal map..
	bool isUsingNormalMap = [&]() -> bool {
		auto iterator = material.materialData.overridenUniforms.find("toUseNormalMap");

		if (iterator == material.materialData.overridenUniforms.end()) {
			return false;
		}

		if (!std::holds_alternative<bool>(iterator->second.value)) {
			return false;
		}

		return std::get<bool>(iterator->second.value);
	}();

	shader.setBool("toUseNormalMap", isUsingNormalMap);

	if (isUsingNormalMap) {
		auto normalMapIterator = material.materialData.overridenUniforms.find("normalMap");

		if (
				normalMapIterator != material.materialData.overridenUniforms.end()
			&&	std::holds_alternative<TypedResourceID<Texture>>(normalMapIterator->second.value)
		) {
			auto [texture, _] = resourceManager.getResource<Texture>(std::get<TypedResourceID<Texture>>(normalMapIterator->second.value));

			if (texture) {
				glBindTextureUnit(0, texture->getTextureId());
				shader.setImageUniform("normalMap", 0);
			}
		}
	}
}

void Renderer::renderNavMesh(dtNavMesh const& mesh) {
	for (int tileNum = 0; tileNum < mesh.getMaxTiles(); ++tileNum) {
		const dtMeshTile* tile = mesh.getTile(tileNum);
		if (!tile->header) continue;

		// dtPolyRef base = mesh.getPolyRefBase(tile);

		for (int i = 0; i < tile->header->polyCount; ++i) {
			const dtPoly* p = &tile->polys[i];
			if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
				continue;

			const dtPolyDetail* polyDetail = &tile->detailMeshes[i];

			for (int j = 0; j < polyDetail->triCount; ++j) {
				const unsigned char* t = &tile->detailTris[(polyDetail->triBase + j) * 4];

				std::array<glm::vec3, 3> triangleVertices;

				// triangle
				for (int k = 0; k < 3; ++k) {
					float* vertex;

					if (t[k] < p->vertCount) {
						vertex = &tile->verts[p->verts[t[k]] * 3];
					}
					else {
						vertex = &tile->detailVerts[(polyDetail->vertBase + t[k] - p->vertCount) * 3];
					}

					triangleVertices[k] = { vertex[0], vertex[1], vertex[2] };
				}

				submitNavMeshTriangle(triangleVertices[0], triangleVertices[1], triangleVertices[2]);
			}
		}
	}
}

void Renderer::renderObjectIds() {
	// Clear object id framebuffer.

	// For some reason, for framebuffers with integer color attachments
	// we need to bind to a shader that writes to integer output
	// even though clear operation does not use our shader at all
	// seems to be a differing / conflicting implementation for NVIDIA GPUs..
	objectIdShader.use();

	constexpr GLuint	nullEntity = entt::null;
	constexpr GLfloat	initialDepth = 1.f;
	constexpr GLint		initialStencilValue = 0;

	glBindVertexArray(mainVAO);

	glDisable(GL_BLEND);
	glDisable(GL_DITHER);

	setDepthMode(DepthTestingMethod::DepthTest);

	glBindFramebuffer(GL_FRAMEBUFFER, objectIdFrameBuffer.fboId());
	glClearNamedFramebufferuiv(objectIdFrameBuffer.fboId(), GL_COLOR, 0, &nullEntity);
	glClearNamedFramebufferfi(objectIdFrameBuffer.fboId(), GL_DEPTH_STENCIL, 0, initialDepth, initialStencilValue);
	
	for (auto&& [entity, transform, entityData, meshRenderer] : registry.view<Transform, EntityData, MeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entity)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		objectIdShader.setMatrix("model", transform.modelMatrix);
		objectIdShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));
		objectIdShader.setUInt("objectId", static_cast<GLuint>(entity));
		
		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			// not rendered, so no point rendering object id..
			if (!obtainMaterial(meshRenderer, mesh)) {
				continue;
			}

			positionsVBO.uploadData(mesh.positions);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}

	for (auto&& [entity, transform, entityData, skinnedMeshRenderer] : registry.view<Transform, EntityData, SkinnedMeshRenderer>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<SkinnedMeshRenderer>(entity)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		objectIdShader.setMatrix("model", transform.modelMatrix);
		objectIdShader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { model->scale, model->scale, model->scale }));
		objectIdShader.setUInt("objectId", static_cast<GLuint>(entity));

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			// not rendered, so no point rendering object id..
			if (!obtainMaterial(skinnedMeshRenderer, mesh)) {
				continue;
			}

			positionsVBO.uploadData(mesh.positions);
			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
}

void Renderer::renderUiObjectIds() {
	// Clear object id framebuffer.

	// For some reason, for framebuffers with integer color attachments
	// we need to bind to a shader that writes to integer output
	// even though clear operation does not use our shader at all
	// seems to be a differing / conflicting implementation for NVIDIA GPUs..
	uiImageObjectIdShader.use();

	constexpr GLuint	nullEntity = entt::null;
	constexpr GLfloat	initialDepth = 1.f;
	constexpr GLint		initialStencilValue = 0;

	glBindVertexArray(mainVAO);

	glDisable(GL_BLEND);
	glDisable(GL_DITHER);

	glBindFramebuffer(GL_FRAMEBUFFER, uiObjectIdFrameBuffer.fboId());
	glClearNamedFramebufferuiv(uiObjectIdFrameBuffer.fboId(), GL_COLOR, 0, &nullEntity);
	glClearNamedFramebufferfi(uiObjectIdFrameBuffer.fboId(), GL_DEPTH_STENCIL, 0, initialDepth, initialStencilValue);

	uiImageObjectIdShader.setMatrix("uiProjection", UIProjection);

	for (auto&& [entity, transform, entityData, image] : registry.view<Transform, EntityData, Image>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Image>(entity)) {
			continue;
		}

		// Get texture resource
		auto [textureAsset, status] = resourceManager.getResource<Texture>(image.texture);
		if (!textureAsset) {
			continue;
		}

		uiImageObjectIdShader.setMatrix("model", transform.modelMatrix);
		uiImageObjectIdShader.setInt("anchorMode", static_cast<int>(image.anchorMode));
		uiImageObjectIdShader.setUInt("objectId", static_cast<GLuint>(entity));

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	uiTextObjectIdShader.use();

	struct Vertex {
		GLfloat x, y;   // screen position
		GLfloat s, t;   // texture coordinates
	};

	glBindVertexArray(textVAO);

	uiTextObjectIdShader.setMatrix("projection", UIProjection);

	// iterate through all characters
	for (auto&& [entity, transform, entityData, text] : registry.view<Transform, EntityData, Text>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Text>(entity)) {
			continue;
		}

		// Retrieves font asset from asset manager.
		auto [font, _] = resourceManager.getResource<Font>(text.font);

		if (!font) {
			continue;
		}

		if (text.text.empty()) {
			continue;
		}

		Font::Atlas const& atlas = font->getAtlas();
		float fontScale = static_cast<float>(text.fontSize) / font->getFontSize();

		float x = transform.position.x;
		float y = transform.position.y;

		std::vector<Vertex> vertices;
		vertices.reserve(text.text.size() * 6); // 6 vertices per char, 4 floats per vertex

		std::string::const_iterator c;
		for (c = text.text.begin(); c != text.text.end(); c++)
		{
			const Font::Character& ch = font->getCharacters().at(*c);

			float xpos = x + ch.bearing.x * fontScale;
			float ypos = y - (ch.size.y - ch.bearing.y) * fontScale;

			float w = ch.size.x * fontScale;
			float h = ch.size.y * fontScale;

			// advance cursors for next glyph 
			x += ch.advance * fontScale;

			// Skip invisible glyphs
			if (w == 0 || h == 0)
				continue;

			float tx = ch.tx;                   // texture X offset in atlas
			float ty = 0.f;                     // texture Y offset in atlas
			float tw = static_cast<float>(ch.size.x) / atlas.width;
			float th = static_cast<float>(ch.size.y) / atlas.height;

			// 6 vertices per quad (2 triangles)
			vertices.push_back({ xpos,     ypos + h, tx,       ty });
			vertices.push_back({ xpos,     ypos,     tx,       ty + th });
			vertices.push_back({ xpos + w, ypos,     tx + tw,  ty + th });

			vertices.push_back({ xpos,     ypos + h, tx,       ty });
			vertices.push_back({ xpos + w, ypos,     tx + tw,  ty + th });
			vertices.push_back({ xpos + w, ypos + h, tx + tw,  ty });
		}

		// Bind font atlas texture once per string
		glBindTexture(GL_TEXTURE_2D, atlas.textureId);

		// Upload vertex data for all glyphs in this string
		textVBO.uploadData(vertices, 0);

		uiTextObjectIdShader.setUInt("objectId", static_cast<GLuint>(entity));

		// Draw all characters in one batched call
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

		// Reset bindings
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Renderer::renderHDRTonemapping(PairFrameBuffer& frameBuffers) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	frameBuffers.swapFrameBuffer();

	// Bind the post-processing framebuffer for LDR output
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	// Set up tone mapping shader
	toneMappingShader.use();
	toneMappingShader.setFloat("exposure", hdrExposure);
	toneMappingShader.setFloat("gamma", 2.2f);
	toneMappingShader.setInt("toneMappingMethod", static_cast<int>(toneMappingMethod));
	toneMappingShader.setBool("toGammaCorrect", toGammaCorrect);

	// Bind the HDR texture from main framebuffer
	glBindTextureUnit(0, frameBuffers.getReadFrameBuffer().textureIds()[0]);
	toneMappingShader.setImageUniform("hdrBuffer", 0);

	// Render fullscreen quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::renderPostProcessing(PairFrameBuffer& frameBuffers) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	frameBuffers.swapFrameBuffer();

	// Bind the post-processing framebuffer for LDR output
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	// Set up tone mapping shader
	postprocessingShader.use();

	// Bind the HDR texture from main framebuffer
	glBindTextureUnit(0, frameBuffers.getReadFrameBuffer().textureIds()[0]);
	postprocessingShader.setImageUniform("scene", 0);

	postprocessingShader.setVec3("offset", chromaticAberration);

	float vignetteDistance = (1 - vignette) * 2.f;
	postprocessingShader.setFloat("vignette", vignetteDistance);

	// Render fullscreen quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

CustomShader* Renderer::setupMaterial(Camera const& camera, Material const& material, Transform const& transform, float scale) {
	// ===========================================================================
	// Retrieve the underlying shader for this material, and verify it's state.
	// ===========================================================================
	TypedResourceID<CustomShader> customShaderId = material.materialData.selectedShader;

	auto&& [customShader, _] = resourceManager.getResource<CustomShader>(customShaderId);

	if (!customShader) {
		return nullptr;
	}

	auto const& shaderOpt = customShader->getShader();
	if (!shaderOpt || !shaderOpt.value().hasCompiled()) {
		return nullptr;
	}

	auto const& shaderData = customShader->customShaderData;

	// ===========================================================================
	// Set rendering fixed pipeline configuration.
	// ===========================================================================

	setBlendMode(material.materialData.blendingConfig);
	setDepthMode(material.materialData.depthTestingMethod);
	setCullMode(material.materialData.cullingConfig);

	Shader const& shader = shaderOpt.value();

	// ===========================================================================
	// Set uniform data..
	// ===========================================================================
	shader.setFloat("timeElapsed", timeElapsed);

	switch (shaderData.pipeline)
	{
	case Pipeline::PBR:
		shader.setVec3("cameraPos", camera.getPos());
		shader.setMatrix("normalMatrix", transform.normalMatrix);

		shader.setFloat("zNear", camera.getNearPlaneDistance());
		shader.setFloat("zFar", camera.getFarPlaneDistance());
		shader.setUVec3("gridSize", { gridSizeX, gridSizeY, gridSizeZ });
		shader.setUVec2("screenDimensions", { gameWidth, gameHeight });
		
		shader.setBool("hasDirectionalLightShadowCaster", hasDirectionalLightShadowCaster);
		
		if (hasDirectionalLightShadowCaster) {
			shader.setMatrix("directionalLightSpaceMatrix", directionalLightViewMatrix);
			shader.setVec3("directionalLightDir", directionalLightDir);
		}

		// both pipeline requires these to be set..
		[[fallthrough]];
	case Pipeline::Color:
		shader.setMatrix("model", transform.modelMatrix);
		shader.setMatrix("localScale", glm::scale(glm::mat4{ 1.f }, { scale, scale, scale }));
		break;
	default:
		assert(false && "Unhandled pipeline.");
		break;
	}

	// we bind to a unused texture unit..
	glBindTextureUnit(0, directionalLightShadowFBO.textureId());
	shader.setImageUniform("shadowMap", 0);

	// We keep track of the number of texture units bound and make sure it doesn't exceed the driver's cap.
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

	// We reserve the very 1st texture unit for shadow map.
	int numOfTextureUnitBound = 1;
	
	for (auto const& [name, overriddenUniformData] : material.materialData.overridenUniforms) {
		std::visit([&](auto&& value) {
			using Type = std::decay_t<decltype(value)>;

			if constexpr (std::same_as<Type, bool>) {
				assert(overriddenUniformData.type == "bool");
				shader.setBool(name, value);
			}
			else if constexpr (std::same_as<Type, int>) {
				assert(overriddenUniformData.type == "int");
				shader.setInt(name, value);
			}
			else if constexpr (std::same_as<Type, unsigned int>) {
				assert(overriddenUniformData.type == "uint");
				shader.setUInt(name, value);
			}
			else if constexpr (std::same_as<Type, float> || std::same_as<Type, NormalizedFloat>) {
				assert(overriddenUniformData.type == "float" || overriddenUniformData.type == "NormalizedFloat");
				shader.setFloat(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec2>) {
				assert(overriddenUniformData.type == "vec2");
				shader.setVec2(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec3> || std::same_as<Type, Color>) {
				assert(overriddenUniformData.type == "vec3" || overriddenUniformData.type == "Color");
				shader.setVec3(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec4> || std::same_as<Type, ColorA>) {
				assert(overriddenUniformData.type == "vec4" || overriddenUniformData.type == "ColorA");
				shader.setVec4(name, value);
			}
			else if constexpr (std::same_as<Type, glm::mat3> || std::same_as<Type, glm::mat4>) {
				assert(overriddenUniformData.type == "mat3" || overriddenUniformData.type == "mat4");
				shader.setMatrix(name, value);
			}
			else if constexpr (std::same_as<Type, TypedResourceID<Texture>>) {
				assert(overriddenUniformData.type == "sampler2D");

				// Setting texture is a way more complicated step.
				// We first retrieve the texture from resource manager..
				auto&& [texture, status] = resourceManager.getResource<Texture>(value);
				
				if (!texture) {
					// We bind an invalid texture..
					auto&& [invalidTexture, __] = resourceManager.getResource<Texture>(INVALID_TEXTURE_ID);
					assert(invalidTexture && "System resource should always be valid.");
					texture = invalidTexture;
				}

				if (numOfTextureUnitBound >= maxTextureUnits) {
					Logger::error("Too many texture units bound. Textures bound: {}, Capacity: {}", numOfTextureUnitBound, maxTextureUnits);
					return;
				}

				// we bind to a unused texture unit..
				glBindTextureUnit(numOfTextureUnitBound, texture->getTextureId());
				shader.setImageUniform(name, numOfTextureUnitBound);

				++numOfTextureUnitBound;
			}
		}, overriddenUniformData.value);
	}

	// Use the shader
	shader.use();

	return customShader;
}

void Renderer::renderMesh(Mesh const& mesh, Pipeline pipeline, MeshType meshType) {
	switch (pipeline)
	{
	case Pipeline::PBR:
		tangentsVBO.uploadData(mesh.tangents);

		[[fallthrough]];
	case Pipeline::Color:
		normalsVBO.uploadData(mesh.normals);
		positionsVBO.uploadData(mesh.positions);
		textureCoordinatesVBO.uploadData(mesh.textureCoordinates);

		break;
	default:
		assert(false && "Unhandled pipeline.");
		break;
	}
	
	if (meshType == MeshType::Skinned) {
		skeletalVBO.uploadData(mesh.vertexWeights);
	}

	EBO.uploadData(mesh.indices);
	glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);
}

void Renderer::setHDRExposure(float exposure) {
	hdrExposure = exposure;
}

float Renderer::getHDRExposure() const {
	return hdrExposure;
}

void Renderer::setToneMappingMethod(ToneMappingMethod method) {
	toneMappingMethod = method;
}

Renderer::ToneMappingMethod Renderer::getToneMappingMethod() const {
	return toneMappingMethod;
}

glm::mat4 const& Renderer::getUIProjection() const {
	return UIProjection;
}

void Renderer::randomiseChromaticAberrationoffset() {
	chromaticAberration = RandomRange::Vec3(glm::vec3{ -0.01f, -0.01f, -0.01f }, glm::vec3{ 0.01f, 0.01f, 0.01f });
}

void Renderer::debugRender() {
	if (engine.toDebugRenderPhysics) {
		debugRenderPhysicsCollider();
	}

	if (engine.toDebugRenderNavMesh) {
		debugRenderNavMesh();
	}

	if (engine.toDebugRenderParticleEmissionShape) {
		debugRenderParticleEmissionShape();
	}

	if (toDebugRenderBoundingVolume) {
		debugRenderBoundingVolume();
	}

	if (toDebugClusters) {
		debugRenderClusters();
	}

	renderDebugSelectedObjects();
}

void Renderer::renderDebugSelectedObjects() {
	debugShader.use();
	debugShader.setVec4("color", { 0.f, 1.0f, 1.0f, 1.0f });
	glVertexArrayVertexBuffer(mainVAO, 0, debugParticleShapeVBO.id(), 0, sizeof(glm::vec3));
	glBindVertexArray(mainVAO);
	glEnable(GL_DEPTH_TEST);

	for (entt::entity entity : selectedEntities) {
		Transform const* transform				= registry.try_get<Transform>(entity);
		Light const* light						= registry.try_get<Light>(entity);
		NavMeshOffLinks const* navMeshOffLinks  = registry.try_get<NavMeshOffLinks>(entity);
		CameraComponent const* cameraComponent	= registry.try_get<CameraComponent>(entity);

		if (!transform) {
			return;
		}

		// Render light's radius of influence
		if (light) {
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, transform->position);

			debugShader.setMatrix("model", model);

			switch (light->type) {
			case Light::Type::Directional:
				debugParticleShapeVBO.uploadData(DebugShapes::Line(glm::vec3{0.f}, transform->front * 2.f));
				glDrawArrays(GL_LINES, 0, 2);
				break;
			case Light::Type::PointLight:
			case Light::Type::Spotlight:
				debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXY(light->radius));
				glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
				debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(light->radius));
				glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
				debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisYZ(light->radius));
				glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
				break;
			}
		}

		// Render navmesh offlinks location
		if (navMeshOffLinks) {
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, navMeshOffLinks->startPoint);
			debugShader.setMatrix("model", model);

			debugParticleShapeVBO.uploadData(DebugShapes::SphereAxisXZ(navMeshOffLinks->radius));
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);

			model = glm::translate(glm::identity<glm::mat4>(), navMeshOffLinks->endPoint);
			debugShader.setMatrix("model", model);

			// same radius, same mesh.
			glDrawArrays(GL_LINE_LOOP, 0, DebugShapes::NUM_DEBUG_CIRCLE_POINTS);
		}

		// Render camera frustum
		if (cameraComponent && cameraComponent->camStatus) {
			// We found our game camera..
			debugShader.setMatrix("model", glm::identity<glm::mat4>());
			auto vertices = DebugShapes::CameraFrustumOutline(transform->position, gameCamera);
			debugParticleShapeVBO.uploadData(vertices);
			glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
		}
	}
}

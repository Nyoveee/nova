#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

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

#undef max

constexpr GLuint clearValue = std::numeric_limits<GLuint>::max();

// 100 MB should be nothing right?
constexpr int AMOUNT_OF_MEMORY_ALLOCATED = 100 * 1024 * 1024;

// we allow a maximum of 10,000 triangle. (honestly some arbritary value lmao)
constexpr int MAX_DEBUG_TRIANGLES = 10000;
constexpr int AMOUNT_OF_MEMORY_FOR_DEBUG = MAX_DEBUG_TRIANGLES * 3 * sizeof(glm::vec3);

// ok right?
constexpr int MAX_NUMBER_OF_LIGHT = 100;

constexpr int MAX_NUMBER_OF_BONES = 255;

Renderer::Renderer(Engine& engine, int gameWidth, int gameHeight) :
	engine						{ engine },
	resourceManager				{ engine.resourceManager },
	registry					{ engine.ecs.registry },
	basicShader					{ "System/Shader/basic.vert",				"System/Shader/basic.frag" },
	standardShader				{ "System/Shader/standard.vert",			"System/Shader/basic.frag" },
	textureShader				{ "System/Shader/standard.vert",			"System/Shader/image.frag" },
	colorShader					{ "System/Shader/standard.vert",			"System/Shader/color.frag" },
	blinnPhongShader			{ "System/Shader/blinnPhong.vert",			"System/Shader/blinnPhong.frag" },
	PBRShader					{ "System/Shader/PBR.vert",					"System/Shader/PBR.frag" },
	gridShader					{ "System/Shader/grid.vert",				"System/Shader/grid.frag" },
	outlineShader				{ "System/Shader/outline.vert",				"System/Shader/outline.frag" },
	debugShader					{ "System/Shader/debug.vert",				"System/Shader/debug.frag" },
	overlayShader				{ "System/Shader/squareOverlay.vert",		"System/Shader/overlay.frag" },
	objectIdShader				{ "System/Shader/standard.vert",			"System/Shader/objectId.frag" },
	skyboxShader				{ "System/Shader/skybox.vert",				"System/Shader/skybox.frag" },
	toneMappingShader			{ "System/Shader/squareOverlay.vert",		"System/Shader/tonemap.frag" },
	particleShader              { "System/Shader/particle.vert",            "System/Shader/particle.frag"},
	skeletalAnimationShader		{ "System/Shader/skeletal.vert",            "System/Shader/PBR.frag"},
	mainVAO						{},
	positionsVBO				{ AMOUNT_OF_MEMORY_ALLOCATED },
	textureCoordinatesVBO		{ AMOUNT_OF_MEMORY_ALLOCATED },
	normalsVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	tangentsVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	skeletalVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	debugPhysicsVBO				{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugNavMeshVBO				{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugParticleShapeVBO       { AMOUNT_OF_MEMORY_FOR_DEBUG },
	EBO							{ AMOUNT_OF_MEMORY_ALLOCATED },

								// we allocate the memory of all light data + space for 1 unsigned int indicating object count.
	pointLightSSBO				{ MAX_NUMBER_OF_LIGHT * sizeof(PointLightData) + alignof(PointLightData)},
	directionalLightSSBO		{ MAX_NUMBER_OF_LIGHT * sizeof(DirectionalLightData) + alignof(DirectionalLightData)},
	spotLightSSBO				{ MAX_NUMBER_OF_LIGHT * sizeof(SpotLightData) + alignof(SpotLightData)},

								// we allocate memory for view and projection matrix.
	sharedUBO					{ 2 * sizeof(glm::mat4) },
	
	bonesSSBO					{ MAX_NUMBER_OF_BONES * sizeof(glm::mat4x4) },
	camera						{},
	numOfPhysicsDebugTriangles	{},
	numOfNavMeshDebugTriangles	{},
	isOnWireframeMode			{},
	hdrExposure					{ 0.25f },
	toneMappingMethod			{ ToneMappingMethod::None },

	mainFrameBuffers			{ {{ gameWidth, gameHeight, { GL_RGBA16F } }, { gameWidth, gameHeight, { GL_RGBA16F } }} },
	physicsDebugFrameBuffer		{ gameWidth, gameHeight, { GL_RGBA8 } },
	objectIdFrameBuffer			{ gameWidth, gameHeight, { GL_R32UI } },
	toGammaCorrect				{ true }
{
	printOpenGLDriverDetails();

	glLineWidth(2.f);

	// ======================================================
	// Prepare shared UBO, that will be used by all shaders. (like view and projection matrix.)
	// ======================================================
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedUBO.id());

	// prepare the light SSBOs. we bind light SSBO to binding point of 0, 1 & 2
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointLightSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, directionalLightSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spotLightSSBO.id());

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

	// enable attributes
	glEnableVertexArrayAttrib(mainVAO, 0);
	glEnableVertexArrayAttrib(mainVAO, 1);
	glEnableVertexArrayAttrib(mainVAO, 2);
	glEnableVertexArrayAttrib(mainVAO, 3);

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
	glVertexArrayAttribBinding(mainVAO, 5, 4);
	glVertexArrayAttribBinding(mainVAO, 6, 4);

	// don't have to enable attributes yet. ;)
	
	// ======================================================
	// Debug VAO configuration
	// - No EBO. A much simpler VAO containing only position.
	// ======================================================
#if 0
	glCreateVertexArrays(1, &debugVAO);
	
	// for this VAO, associate bindingIndex 0 with this VBO. 
	constexpr GLuint debugBindingIndex = 0;

	glVertexArrayVertexBuffer(debugVAO, debugBindingIndex, debugPhysicsVBO.id(), 0, sizeof(SimpleVertex));

	// associate attribute index 0 with the respective attribute properties.
	glVertexArrayAttribFormat(debugVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(SimpleVertex, pos));

	// enable attribute
	glEnableVertexArrayAttrib(debugVAO, 0);

	// associate vertex attribute 0 with binding index 1.
	glVertexArrayAttribBinding(debugVAO, 0, debugBindingIndex);
	// ======================================================
	// Particle VAO configuration
	// ======================================================
	glCreateVertexArrays(1, &particleVAO);
	constexpr GLuint particleBindingIndex = 0;

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(particleVAO, EBO.id());

	// Associate binding index 0 with this vbo for particleVAO
	glVertexArrayVertexBuffer(particleVAO, particleBindingIndex, mainVBO.id(), 0, sizeof(ParticleVertex));

	// Associate Attribute Indexes with their properties
	glVertexArrayAttribFormat(particleVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, localPos));
	glVertexArrayAttribFormat(particleVAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, worldPos));
	glVertexArrayAttribFormat(particleVAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, texCoord));
	glVertexArrayAttribFormat(particleVAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, color));
	glVertexArrayAttribFormat(particleVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, rotation));

	// Enable attributes
	glEnableVertexArrayAttrib(particleVAO, 0);
	glEnableVertexArrayAttrib(particleVAO, 1);
	glEnableVertexArrayAttrib(particleVAO, 2);
	glEnableVertexArrayAttrib(particleVAO, 3);
	glEnableVertexArrayAttrib(particleVAO, 4);

	// Associate vertex attributes with binding index 0
	glVertexArrayAttribBinding(particleVAO, 0, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 1, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 2, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 3, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 4, particleBindingIndex);
#endif
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &mainVAO);
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

void Renderer::update([[maybe_unused]] float dt) {
	ZoneScoped;
}

void Renderer::render(bool toRenderDebugPhysics, bool toRenderDebugNavMesh, bool toRenderDebugParticleEmissionShape) {
	ZoneScoped;
	prepareRendering();

	renderSkyBox();
	renderModels();
	renderSkinnedModels();
	renderParticles();

	if (toRenderDebugPhysics) {
		debugRenderPhysicsCollider();
	}
	if (toRenderDebugNavMesh) {
		debugRenderNavMesh();
	}
	if (toRenderDebugParticleEmissionShape) {
		debugRenderParticleEmissionShape();
	}


	// ======= Post Processing =======
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Apply HDR tone mapping + gamma correction post-processing
	renderHDRTonemapping();

	// Bind back to default FBO for ImGui or Nova-Game to work on.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderToDefaultFBO() {
	overlayShader.use();
	overlayShader.setImageUniform("overlay", 0);
	glBindTextureUnit(0, getMainFrameBufferTexture());

	// VBO-less draw.
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint Renderer::getMainFrameBufferTexture() const {
	return getActiveMainFrameBuffer().textureIds()[0];
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

void Renderer::recompileShaders() {
	blinnPhongShader.compile();
	PBRShader.compile();
	skyboxShader.compile();
	toneMappingShader.compile();
	particleShader.compile();
	skeletalAnimationShader.compile();
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

	// enable wireframe mode only for debug overlay.
	if (!isOnWireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	debugShader.use();
	debugShader.setVec4("color", { 0.f, 1.f, 0.f, 1.f });
	glDrawArrays(GL_TRIANGLES, 0, numOfPhysicsDebugTriangles * 3);

	glDisable(GL_DEPTH_TEST);
	overlayShader.use();
	numOfPhysicsDebugTriangles = 0;
	
	// ================================================
	// 2. We overlay this resulting debug shapes into our main FBO, with alpha blending.
	// (so post processing)
	// ================================================

	// disable wireframe mode, restoring to normal fill
	if (!isOnWireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	setBlendMode(CustomShader::BlendingConfig::AlphaBlending);
	glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());
	
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
	glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());

	glDisable(GL_STENCIL_TEST);
	setBlendMode(CustomShader::BlendingConfig::AlphaBlending);
	glEnable(GL_DEPTH_TEST);

	debugShader.use();
	debugShader.setVec4("color", { 0.f, 0.8f, 0.8f, 0.5f });
	glDrawArrays(GL_TRIANGLES, 0, numOfNavMeshDebugTriangles * 3);

	glDisable(GL_DEPTH_TEST);
	setBlendMode(CustomShader::BlendingConfig::Disabled);
	numOfNavMeshDebugTriangles = 0;
}

void Renderer::debugRenderParticleEmissionShape()
{
	debugShader.use();
	debugShader.setVec4("color", { 0.f,1.0f,1.0f,1.0f });
	glVertexArrayVertexBuffer(mainVAO, 0, debugParticleShapeVBO.id(), 0, sizeof(glm::vec3));

	glBindVertexArray(mainVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());
	glEnable(GL_DEPTH_TEST);
	setBlendMode(CustomShader::BlendingConfig::AlphaBlending);

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

void Renderer::submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3) {
	if (numOfPhysicsDebugTriangles > MAX_DEBUG_TRIANGLES) {
		std::cerr << "too much triangles!\n";
		return;
	}

	debugPhysicsVBO.uploadData(std::vector<glm::vec3>{ { vertice1 }, { vertice2 }, { vertice3 } }, 3 * numOfPhysicsDebugTriangles * sizeof(glm::vec3));
	++numOfPhysicsDebugTriangles;
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
	ZoneScopedC(tracy::Color::PaleVioletRed1);
	// =================================================================
	// Configure pre rendering settings
	// =================================================================
	
	// of course.
	glEnable(GL_DEPTH_TEST);
	glStencilMask(0xFF);
	glDisable(GL_BLEND);

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

	// Clear both main framebuffers.
	for (auto&& framebuffer : mainFrameBuffers) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fboId());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	// We choose 1 as the active index because we last bind to the last element of the array above^
	
	// Reset main frame buffer indices..
	mainFrameBufferActiveIndex = 1;
	mainFrameBufferReadIndex = 0;

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

	// ==== Set up camera position uniform ====
	blinnPhongShader.setVec3("cameraPos", camera.getPos());
	PBRShader.setVec3("cameraPos", camera.getPos());
	skeletalAnimationShader.setVec3("cameraPos", camera.getPos());

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
	for (auto&& [entity, transform, emitter] : registry.view<Transform, ParticleEmitter>().each())
	{
		if (emitter.lightIntensity <= 0)
			continue;
		for (Particle const& particle : emitter.particles) {
			if (numOfPtLights >= MAX_NUMBER_OF_LIGHT) {
				Logger::warn("Unable to add more particle lights, max number of point lights reached!");
				break;
			}
			pointLightData[numOfPtLights++] = {
				particle.position,
				glm::vec3{ particle.currentColor } *emitter.lightIntensity,
				emitter.lightattenuation
			};
		}
		if (numOfPtLights >= MAX_NUMBER_OF_LIGHT)
			break;
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
	ZoneScopedC(tracy::Color::PaleVioletRed1);
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

#if 0
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
				setupPBRShader(*material);
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
#endif
	}

	glDisable(GL_CULL_FACE);
}

void Renderer::renderSkinnedModels() {
	ZoneScopedC(tracy::Color::PaleVioletRed1);

	// enable back face culling for our 3d models..
	glEnable(GL_CULL_FACE);

	// enable skeletal animation vertex attribute..
	glEnableVertexArrayAttrib(mainVAO, 5);
	glEnableVertexArrayAttrib(mainVAO, 6);

	for (auto&& [entity, transform, skinnedMeshRenderer] : registry.view<Transform, SkinnedMeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

#if 0
		// Setting model specific uniform..
		skeletalAnimationShader.setMatrix("model", transform.modelMatrix);
		skeletalAnimationShader.setMatrix("normalMatrix", transform.normalMatrix);

		objectIdShader.setMatrix("model", transform.modelMatrix);
		objectIdShader.setUInt("objectId", static_cast<GLuint>(entity));

		// upload all bone matrices..
		glNamedBufferSubData(bonesSSBO.id(), 0, skinnedMeshRenderer.bonesFinalMatrices.size() * sizeof(glm::mat4x4), skinnedMeshRenderer.bonesFinalMatrices.data());

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			Material const* material = obtainMaterial(skinnedMeshRenderer, mesh);

			if (!material) {
				continue;
			}

			// TEMP!! WE FORCE EVERY SKINNED MESH RENDERER TO USE A PBR FOR NOW.
			setupSkeletalShader(*material);

			// time to draw!
			mainVBO.uploadData(mesh.vertices);
			skeletalVBO.uploadData(mesh.vertexWeights);

			EBO.uploadData(mesh.indices);
			glDrawElements(GL_TRIANGLES, mesh.numOfTriangles * 3, GL_UNSIGNED_INT, 0);

			// render object id into object id FBO.
			renderObjectId(mesh.numOfTriangles * 3);
		}
#endif 
	}

	glDisable(GL_CULL_FACE);

	glDisableVertexArrayAttrib(mainVAO, 5);
	glDisableVertexArrayAttrib(mainVAO, 6);
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
#if 0
	glBindVertexArray(particleVAO);

	setBlendMode(Renderer::BlendingConfig::AlphaBlending);
	particleShader.use();

	// Disable writing to depth buffer for particles
	glDepthMask(GL_FALSE);

	for (auto&& [entity, transform, emitter] : registry.view<Transform, ParticleEmitter>().each()) {
		auto&& [texture, result] = resourceManager.getResource<Texture>(emitter.texture);
		if (!texture)
			continue;
		glBindTextureUnit(0, texture->getTextureId());
		const glm::vec3 vertexPos[4]{
			glm::vec3(-1, -1, 0),	// bottom left
			glm::vec3(1, -1, 0),	// bottom right
			glm::vec3(1, 1, 0),	// top right
			glm::vec3(-1, 1, 0) 	// top left
		};
		const glm::vec2 textureCoordinates[4]{
			glm::vec2(0, 0),
			glm::vec2(1, 0),
			glm::vec2(1, 1),
			glm::vec2(0, 1)
		};
		const int squareIndices[6]{ 0, 2, 1, 2, 0, 3 };
		std::vector<ParticleVertex> particleVertexes;
		std::vector<unsigned int> indices;

		for (size_t i{}; i < std::size(emitter.particles); ++i) {
			ParticleVertex particleVertex;
			// Add to batch
			for (int j{}; j < 4; ++j) {
				particleVertex.localPos = vertexPos[j] * emitter.particles[i].currentSize;
				particleVertex.worldPos = emitter.particles[i].position;
				particleVertex.texCoord = textureCoordinates[j];
				particleVertex.color = emitter.particles[i].currentColor;
				particleVertex.rotation = emitter.particles[i].rotation;
				particleVertexes.push_back(particleVertex);
			}
			for (int j{}; j < 6; ++j)
				indices.push_back(squareIndices[j] + static_cast<int>(i) * 4);
			
		
		}
		mainVBO.uploadData(particleVertexes);
		EBO.uploadData(indices);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	}
	// Renable Depth Writing for other rendering
	glDepthMask(GL_TRUE);
#endif
}

#if 0
void Renderer::renderObjectId(GLsizei count) {
	glDisable(GL_DITHER);

	glBindFramebuffer(GL_FRAMEBUFFER, objectIdFrameBuffer.fboId());
	objectIdShader.use();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

	// after rendering object id, swap back to main frame buffer..	
	// this function is invoked when rendering models..
	glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());

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
	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(albedo);

			if (!texture) {
				// fallback..
				PBRShader.setBool("isUsingAlbedoMap", false);
				PBRShader.setVec3("albedo", { 0.2f, 0.2f, 0.2f });
			}
			else {
				PBRShader.setBool("isUsingAlbedoMap", true);
				glBindTextureUnit(0, texture->getTextureId());
				PBRShader.setImageUniform("albedoMap", 0);
			}
		}
		else /* it's Color */ {
			PBRShader.setBool("isUsingAlbedoMap", false);
			PBRShader.setVec3("albedo", albedo);
		}

		}, material.albedo);

	// Handle normal map
	if (material.normalMap) {
		auto&& [normalMap, result] = resourceManager.getResource<Texture>(material.normalMap.value());

		if (normalMap) {
			PBRShader.setBool("isUsingNormalMap", true);
			glBindTextureUnit(1, normalMap->getTextureId());
			PBRShader.setImageUniform("normalMap", 1);
		}
	}
	else {
		PBRShader.setBool("isUsingNormalMap", false);
	}

	std::visit([&](auto&& config) {
		using T = std::decay_t<decltype(config)>;

		// Packed map
		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(config);
			if (!texture) {
				PBRShader.setBool("isUsingPackedTextureMap", false);
				PBRShader.setFloat("material.roughness", 0.2f);
				PBRShader.setFloat("material.metallic", 0.2f);
				PBRShader.setFloat("material.occulusion", 0.2f);
			}
			else {
				PBRShader.setBool("isUsingPackedTextureMap", true);
				glBindTextureUnit(2, texture->getTextureId());
				PBRShader.setImageUniform("packedMap", 2);
			}
		}
		// Constants
		else {
			PBRShader.setBool("isUsingPackedTextureMap", false);
			PBRShader.setFloat("material.roughness", config.roughness);
			PBRShader.setFloat("material.metallic", config.metallic);
			PBRShader.setFloat("material.occulusion", config.occulusion);
		}

		}, material.config);

	PBRShader.setFloat("ambientFactor", material.ambient);

	PBRShader.use();
}

void Renderer::setupSkeletalShader(Material const& material) {
	// Handle albedo.
	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(albedo);

			if (!texture) {
				// fallback..
				skeletalAnimationShader.setBool("isUsingAlbedoMap", false);
				skeletalAnimationShader.setVec3("albedo", { 0.2f, 0.2f, 0.2f });
			}
			else {
				skeletalAnimationShader.setBool("isUsingAlbedoMap", true);
				glBindTextureUnit(0, texture->getTextureId());
				skeletalAnimationShader.setImageUniform("albedoMap", 0);
			}
		}
		else /* it's Color */ {
			skeletalAnimationShader.setBool("isUsingAlbedoMap", false);
			skeletalAnimationShader.setVec3("albedo", albedo);
		}

		}, material.albedo);

	// Handle normal map
	if (material.normalMap) {
		auto&& [normalMap, result] = resourceManager.getResource<Texture>(material.normalMap.value());

		if (normalMap) {
			skeletalAnimationShader.setBool("isUsingNormalMap", true);
			glBindTextureUnit(1, normalMap->getTextureId());
			skeletalAnimationShader.setImageUniform("normalMap", 1);
		}
	}
	else {
		skeletalAnimationShader.setBool("isUsingNormalMap", false);
	}

	std::visit([&](auto&& config) {
		using T = std::decay_t<decltype(config)>;

		// Packed map
		if constexpr (std::same_as<T, ResourceID>) {
			auto&& [texture, result] = resourceManager.getResource<Texture>(config);
			if (!texture) {
				skeletalAnimationShader.setBool("isUsingPackedTextureMap", false);
				skeletalAnimationShader.setFloat("material.roughness", 0.2f);
				skeletalAnimationShader.setFloat("material.metallic", 0.2f);
				skeletalAnimationShader.setFloat("material.occulusion", 0.2f);
			}
			else {
				skeletalAnimationShader.setBool("isUsingPackedTextureMap", true);
				glBindTextureUnit(2, texture->getTextureId());
				skeletalAnimationShader.setImageUniform("packedMap", 2);
			}
		}
		// Constants
		else {
			skeletalAnimationShader.setBool("isUsingPackedTextureMap", false);
			skeletalAnimationShader.setFloat("material.roughness", config.roughness);
			skeletalAnimationShader.setFloat("material.metallic", config.metallic);
			skeletalAnimationShader.setFloat("material.occulusion", config.occulusion);
		}

	}, material.config);

	skeletalAnimationShader.setFloat("ambientFactor", material.ambient);

	skeletalAnimationShader.use();
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

#endif

void Renderer::setModelUniforms(Transform const& transform, entt::entity entity) {
	blinnPhongShader.setMatrix("model", transform.modelMatrix);
	blinnPhongShader.setMatrix("normalMatrix", transform.normalMatrix);
	PBRShader.setMatrix("model", transform.modelMatrix);
	PBRShader.setMatrix("normalMatrix", transform.normalMatrix);

	colorShader.setMatrix("model", transform.modelMatrix);

	textureShader.setMatrix("model", transform.modelMatrix);

	objectIdShader.setMatrix("model", transform.modelMatrix);
	objectIdShader.setUInt("objectId", static_cast<GLuint>(entity));
}

#if 0
Material const* Renderer::obtainMaterial(MeshRenderer const& meshRenderer, Mesh const& mesh) {
	auto iterator = meshRenderer.materials.find(mesh.materialName);

	if (iterator == meshRenderer.materials.end()) {
		Logger::warn("This shouldn't happen. Material in component does not correspond to material on mesh.");
		return nullptr;
	}

	// We have successfully retrieved the materials. Let's retrieve the individual components.
	auto&& [__, material] = *iterator;
	return &material;
}

Material const* Renderer::obtainMaterial(SkinnedMeshRenderer const& skinnedMeshRenderer, Mesh const& mesh) {
	auto iterator = skinnedMeshRenderer.materials.find(mesh.materialName);

	if (iterator == skinnedMeshRenderer.materials.end()) {
		Logger::warn("This shouldn't happen. Material in component does not correspond to material on mesh.");
		return nullptr;
	}

	// We have successfully retrieved the materials. Let's retrieve the individual components.
	auto&& [__, material] = *iterator;
	return &material;
}
#endif

void Renderer::setBlendMode(CustomShader::BlendingConfig configuration) {
	switch (configuration) {
		using enum CustomShader::BlendingConfig;
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
	default:
		assert(false && "Forget to handle other case.");
		break;
	}
}

void Renderer::setDepthMode(CustomShader::DepthTestingMethod configuration) {
	switch (configuration) {
		using enum CustomShader::DepthTestingMethod;
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

FrameBuffer const& Renderer::getActiveMainFrameBuffer() const {
	return mainFrameBuffers[mainFrameBufferActiveIndex];
}

FrameBuffer const& Renderer::getReadMainFrameBuffer() const {
	return mainFrameBuffers[mainFrameBufferReadIndex];
}

void Renderer::swapMainFrameBuffers() {
	if (mainFrameBufferActiveIndex == 0) {
		mainFrameBufferActiveIndex = 1;
		mainFrameBufferReadIndex = 0;
	}
	else if (mainFrameBufferActiveIndex == 1){
		mainFrameBufferActiveIndex = 0;
		mainFrameBufferReadIndex = 1;
	}
	else {
		assert(false && "Invalid index.");
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

void Renderer::renderHDRTonemapping() {
	ZoneScoped;

	swapMainFrameBuffers();

	// Bind the post-processing framebuffer for LDR output
	glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());

	// Set up tone mapping shader
	toneMappingShader.use();
	toneMappingShader.setFloat("exposure", hdrExposure);
	toneMappingShader.setFloat("gamma", 2.2f);
	toneMappingShader.setInt("toneMappingMethod", static_cast<int>(toneMappingMethod));
	toneMappingShader.setBool("toGammaCorrect", toGammaCorrect);

	// Bind the HDR texture from main framebuffer
	glBindTextureUnit(0, getReadMainFrameBuffer().textureIds()[0]);
	toneMappingShader.setImageUniform("hdrBuffer", 0);

	// Render fullscreen triangle (more efficient than quad)
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool Renderer::setupMaterial(Material const& material) {
	// ===========================================================================
	// Retrieve the underlying shader for this material, and verify it's state.
	// ===========================================================================
	TypedResourceID<CustomShader> customShaderId = material.materialData.selectedShader;

	auto&& [customShader, _] = resourceManager.getResource<CustomShader>(customShaderId);

	if (!customShader) {
		return false;
	}

	auto const& shaderOpt = customShader->getShader();
	if (!shaderOpt || !shaderOpt.value().hasCompiled()) {
		return false;
	}

	auto const& shaderData = customShader->customShaderData;

	// ===========================================================================
	// Set rendering fixed pipeline configuration.
	// ===========================================================================

	setBlendMode(shaderData.blendingConfig);
	setDepthMode(shaderData.depthTestingMethod);

	Shader const& shader = shaderOpt.value();

	// ===========================================================================
	// Set uniform data..
	// ===========================================================================
	
	// We keep track of the number of texture units bound and make sure it doesn't exceed the driver's cap.
	GLint maxTextureUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

	int numOfTextureUnitBound = 0;
	
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
			else if constexpr (std::same_as<Type, float>) {
				assert(overriddenUniformData.type == "float");
				shader.setFloat(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec2>) {
				assert(overriddenUniformData.type == "vec2");
				shader.setVec2(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec3>) {
				assert(overriddenUniformData.type == "vec3");
				shader.setVec3(name, value);
			}
			else if constexpr (std::same_as<Type, glm::vec4>) {
				assert(overriddenUniformData.type == "vec4");
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
				auto&& [texture, _] = resourceManager.getResource<Texture>(value);
				
				if (!texture) {
					// @TODO: Add error texture..
					return;
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

	return true;
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

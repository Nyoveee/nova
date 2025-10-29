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

#include "systemResource.h"

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
	textShader					{ "System/Shader/text.vert",				"System/Shader/text.frag"},
	skeletalAnimationShader		{ "System/Shader/skeletal.vert",            "System/Shader/PBR.frag"},
	mainVAO						{},
	positionsVBO				{ AMOUNT_OF_MEMORY_ALLOCATED },
	textureCoordinatesVBO		{ AMOUNT_OF_MEMORY_ALLOCATED },
	normalsVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	tangentsVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	skeletalVBO					{ AMOUNT_OF_MEMORY_ALLOCATED },
	particleVBO                 { AMOUNT_OF_MEMORY_ALLOCATED },
	debugPhysicsVBO				{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugNavMeshVBO				{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	debugParticleShapeVBO       { AMOUNT_OF_MEMORY_FOR_DEBUG },
	textVBO						{ AMOUNT_OF_MEMORY_FOR_DEBUG },
	EBO							{ AMOUNT_OF_MEMORY_ALLOCATED },

								// we allocate the memory of all light data + space for 1 unsigned int indicating object count.
	pointLightSSBO				{ MAX_NUMBER_OF_LIGHT * sizeof(PointLightData) + alignof(PointLightData)},
	directionalLightSSBO		{ MAX_NUMBER_OF_LIGHT * sizeof(DirectionalLightData) + alignof(DirectionalLightData)},
	spotLightSSBO				{ MAX_NUMBER_OF_LIGHT * sizeof(SpotLightData) + alignof(SpotLightData)},

								// we allocate memory for view and projection matrix.
	sharedUBO					{ 2 * sizeof(glm::mat4) },
	
								// we allocate the memory of all bone data + space for 1 unsigned int indicating true or false (whether the current invocation is a skinned meshrenderer).
	bonesSSBO					{ MAX_NUMBER_OF_BONES * sizeof(glm::mat4x4) + alignof(glm::vec4) },
	editorCamera				{},
	gameCamera					{},
	numOfPhysicsDebugTriangles	{},
	numOfNavMeshDebugTriangles	{},
	isOnWireframeMode			{},
	hdrExposure					{ 0.25f },
	toneMappingMethod			{ ToneMappingMethod::None },

	editorMainFrameBuffer		{ gameWidth, gameHeight, { GL_RGBA16F } },
	gameMainFrameBuffer			{ gameWidth, gameHeight, { GL_RGBA16F } },
	physicsDebugFrameBuffer		{ gameWidth, gameHeight, { GL_RGBA8 } },
	objectIdFrameBuffer			{ gameWidth, gameHeight, { GL_R32UI } },
	toGammaCorrect				{ true },
	UIProjection				{ glm::ortho(0.0f, static_cast<float>(gameWidth), 0.0f, static_cast<float>(gameHeight)) }
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
	constexpr GLuint particleBindingIndex = 0;

	// Bind this EBO to this VAO.
	glVertexArrayElementBuffer(particleVAO, EBO.id());

	// Associate binding index 0 with this vbo for particleVAO
	glVertexArrayVertexBuffer(particleVAO, particleBindingIndex, particleVBO.id(), 0, sizeof(ParticleVertex));

	// Associate Attribute Indexes with their properties
	glVertexArrayAttribFormat(particleVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, localPos));
	glVertexArrayAttribFormat(particleVAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, worldPos));
	glVertexArrayAttribFormat(particleVAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, texCoord));
	glVertexArrayAttribFormat(particleVAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, color));
	glVertexArrayAttribFormat(particleVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, rotation));

	// Associate vertex attributes with binding index 0
	glVertexArrayAttribBinding(particleVAO, 0, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 1, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 2, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 3, particleBindingIndex);
	glVertexArrayAttribBinding(particleVAO, 4, particleBindingIndex);

	// Enable Attributes
	glEnableVertexArrayAttrib(particleVAO, 0);
	glEnableVertexArrayAttrib(particleVAO, 1);
	glEnableVertexArrayAttrib(particleVAO, 2);
	glEnableVertexArrayAttrib(particleVAO, 3);
	glEnableVertexArrayAttrib(particleVAO, 4);

}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &mainVAO);
	glDeleteVertexArrays(1, &textVAO);
	glDeleteVertexArrays(1, &particleVAO);
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

void Renderer::renderMain(RenderConfig renderConfig) {
	ZoneScoped;
	prepareRendering();

	// The renderer 
	switch (renderConfig)
	{
	// ===============================================
	// In this case, we focus on rendering to the editor's FBO.
	// ===============================================
	case RenderConfig::Editor:
		// Main render function
		render(editorMainFrameBuffer, editorCamera);

		// ===============================================
		// Debug rendering + object ids for editor..
		// ===============================================
		if (engine.toDebugRenderPhysics) {
			debugRenderPhysicsCollider();
		}

		if (engine.toDebugRenderNavMesh) {
			debugRenderNavMesh();
		}

		if (engine.toDebugRenderParticleEmissionShape) {
			debugRenderParticleEmissionShape();
		}

		// after debug rendering.. bind main position VBO back to VAO..
		glVertexArrayVertexBuffer(mainVAO, 0, positionsVBO.id(), 0, sizeof(glm::vec3));

		renderObjectIds();

	// ===============================================
	// In this case, we focus on rendering to the game's FBO.
	// ===============================================
	// Editor mode would also need to do the same work to render into the game's FBO..
	[[fallthrough]];
	case RenderConfig::Game:
		// Main render function
		render(gameMainFrameBuffer, gameCamera);

		// Bind back to default FBO for ImGui or Nova-Game to work on.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// only render to default FBO if it's truly game mode.
		if (renderConfig == RenderConfig::Game) {
			renderToDefaultFBO();
		}

		break;
	default:
		assert(false && "Forget to account for a case.");
		break;
	}
}

void Renderer::render(PairFrameBuffer& frameBuffers, Camera const& camera) {
	// We clear this pair frame buffer..
	frameBuffers.clearFrameBuffers();

	// We bind to the active framebuffer for majority of the in game rendering..
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.getActiveFrameBuffer().fboId());

	// We upload camera data to the UBO..
	glNamedBufferSubData(sharedUBO.id(), 0, sizeof(glm::mat4x4), glm::value_ptr(camera.view()));
	glNamedBufferSubData(sharedUBO.id(), sizeof(glm::mat4x4), sizeof(glm::mat4x4), glm::value_ptr(camera.projection()));

	// We render individual game objects..
	renderSkyBox();
	renderModels(camera);
	renderSkinnedModels(camera);
	renderParticles();
	renderTexts();

	// ======= Post Processing =======
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Apply HDR tone mapping + gamma correction post-processing
	renderHDRTonemapping(frameBuffers);
}

void Renderer::renderToDefaultFBO() {
	overlayShader.use();
	overlayShader.setImageUniform("overlay", 0);
	glBindTextureUnit(0, getGameFrameBufferTexture());

	// VBO-less draw.
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint Renderer::getEditorFrameBufferTexture() const {
	return editorMainFrameBuffer.getActiveFrameBuffer().textureIds()[0];
}

GLuint Renderer::getGameFrameBufferTexture() const {
	return gameMainFrameBuffer.getActiveFrameBuffer().textureIds()[0];
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
	blinnPhongShader.compile();
	PBRShader.compile();
	skyboxShader.compile();
	textShader.compile();
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

	// glBindFramebuffer(GL_FRAMEBUFFER, getActiveMainFrameBuffer().fboId());
	
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
	glEnable(GL_DITHER);

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

	// =================================================================
	// Set up the uniforms for my respective shaders
	// Note: calling shader.use() before setting uniforms is redundant because we are using DSA.
	// =================================================================

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

void Renderer::renderModels(Camera const& camera) {
	ZoneScopedC(tracy::Color::PaleVioletRed1);	

	glEnable(GL_CULL_FACE);

	// indicate that this is NOT a skinned mesh renderer..
	static const unsigned int isNotASkinnedMeshRenderer = 0;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isNotASkinnedMeshRenderer);

	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(meshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}

		// Draw every mesh of a given model.
		for (auto const& mesh : model->meshes) {
			Material const* material = obtainMaterial(meshRenderer, mesh);

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

	glDisable(GL_CULL_FACE);
}

void Renderer::renderTexts()
{
	struct Vertex {
		GLfloat x, y;   // screen position
		GLfloat s, t;   // texture coordinates
	};

	// activate corresponding render state	
	textShader.use();
	textShader.setMatrix("projection", UIProjection);

	setBlendMode(CustomShader::BlendingConfig::AlphaBlending);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// iterate through all characters
	for (auto&& [entity, transform, text] : registry.view<Transform, Text>().each()) {
		// Retrieves font asset from asset manager.
		auto [font, _] = resourceManager.getResource<Font>(text.font);

		if (!font) {
			continue;
		}

		if (text.text.empty()) {
			continue;
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

		// Reset bindings
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(mainVAO);
	glDisable(GL_BLEND);
}

void Renderer::renderSkinnedModels(Camera const& camera) {
	ZoneScopedC(tracy::Color::PaleVioletRed1);

	// enable back face culling for our 3d models..
	glEnable(GL_CULL_FACE);

	// indicate that this is a skinned mesh renderer..
	static const unsigned int isSkinnedMeshRenderer = 1;
	glNamedBufferSubData(bonesSSBO.id(), 0, sizeof(glm::vec4), &isSkinnedMeshRenderer);

	for (auto&& [entity, transform, skinnedMeshRenderer] : registry.view<Transform, SkinnedMeshRenderer>().each()) {
		// Retrieves model asset from asset manager.
		auto [model, _] = resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);

		if (!model) {
			// missing model.
			continue;
		}
		
		// upload all bone matrices..
		glNamedBufferSubData(bonesSSBO.id(), sizeof(glm::vec4), skinnedMeshRenderer.bonesFinalMatrices.size() * sizeof(glm::mat4x4), skinnedMeshRenderer.bonesFinalMatrices.data());

		// Draw every mesh of a given model.
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
	setBlendMode(CustomShader::BlendingConfig::AlphaBlending);
	particleShader.use();

	// Disable writing to depth buffer for particles
	glDepthMask(GL_FALSE);

	for (auto&& [entity, transform, emitter] : registry.view<Transform, ParticleEmitter>().each()) {
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
		int i{};
		auto renderParticleBatch = [&](std::vector<Particle> const &particles) {
			for (Particle const &particle: particles) {
				auto&& [texture, result] = resourceManager.getResource<Texture>(particle.texture);
				if (!texture)
					continue;
				glBindTextureUnit(0, texture->getTextureId());
				ParticleVertex particleVertex;
				// Add to batch
				for (int j{}; j < 4; ++j) {
					particleVertex.localPos = vertexPos[j] * particle.currentSize;
					particleVertex.worldPos = particle.position;
					particleVertex.texCoord = textureCoordinates[j];
					particleVertex.color = particle.currentColor;
					particleVertex.rotation = particle.rotation;
					particleVertexes.push_back(particleVertex);
				}
				for (int j{}; j < 6; ++j)
					indices.push_back(squareIndices[j] + i * 4);
				++i;
			}
			particleVBO.uploadData(particleVertexes);
			EBO.uploadData(indices);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		};
		renderParticleBatch(emitter.trailParticles);
		renderParticleBatch(emitter.particles);
		
		
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
	glBindFramebuffer(GL_FRAMEBUFFER, objectIdFrameBuffer.fboId());
	glClearNamedFramebufferuiv(objectIdFrameBuffer.fboId(), GL_COLOR, 0, &nullEntity);
	glClearNamedFramebufferfi(objectIdFrameBuffer.fboId(), GL_DEPTH_STENCIL, 0, initialDepth, initialStencilValue);
	
	for (auto&& [entity, transform, meshRenderer] : registry.view<Transform, MeshRenderer>().each()) {
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

	for (auto&& [entity, transform, skinnedMeshRenderer] : registry.view<Transform, SkinnedMeshRenderer>().each()) {
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

void Renderer::renderHDRTonemapping(PairFrameBuffer& frameBuffers) {
	ZoneScoped;

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

	// Render fullscreen triangle (more efficient than quad)
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

	setBlendMode(shaderData.blendingConfig);
	setDepthMode(shaderData.depthTestingMethod);

	Shader const& shader = shaderOpt.value();

	// ===========================================================================
	// Set uniform data..
	// ===========================================================================
	switch (shaderData.pipeline)
	{
	case Pipeline::PBR:
		shader.setVec3("cameraPos", camera.getPos());
		shader.setMatrix("normalMatrix", transform.normalMatrix);

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
				auto&& [texture, _] = resourceManager.getResource<Texture>(value);
				
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
		normalsVBO.uploadData(mesh.normals);
		tangentsVBO.uploadData(mesh.tangents);

		[[fallthrough]];
	case Pipeline::Color:
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

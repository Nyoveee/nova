#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "bufferObject.h"
#include "framebuffer.h"
#include "ECS/ECS.h"
#include "component.h"
#include "vertex.h"

#include "model.h"
#include "cubemap.h"

#include "Detour/Detour/DetourNavMesh.h"

class Engine;
class ResourceManager;

class Renderer {
public:
	enum class BlendingConfig {
		AlphaBlending,
		AdditiveBlending,
		PureAdditiveBlending,
		PremultipliedAlpha,
		Disabled
	};

	enum class ToneMappingMethod {
		Exposure,
		Reinhard,
		ACES,
		None
	};

public:
	Renderer(Engine& engine, int gameWidth, int gameHeight);

	~Renderer();
	Renderer(Renderer const& other)				= delete;
	Renderer(Renderer&& other)					= delete;
	Renderer& operator=(Renderer const& other)	= delete;
	Renderer& operator=(Renderer&& other)		= delete;

public:
	void update(float dt);

	void render(bool toRenderDebugPhysics, bool toRenderDebugNavMesh, bool toRenderDebugParticleEmissionShape);
	void renderToDefaultFBO();

public:
	// =============================================
	// Public facing API.
	// =============================================

	// get the main texture of the main frame buffer.
	ENGINE_DLL_API GLuint getMainFrameBufferTexture() const;
	ENGINE_DLL_API void enableWireframeMode(bool toEnable);

	// gets object id from color attachment 1 of the main framebuffer.
	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	ENGINE_DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

	ENGINE_DLL_API Camera& getCamera();
	ENGINE_DLL_API Camera& getGameViewport();

	ENGINE_DLL_API Camera const& getCamera() const;
	ENGINE_DLL_API Camera const& getGameViewport() const;

	ENGINE_DLL_API void setGameViewport(Camera& cam);


	// most probably for ease of development.
	ENGINE_DLL_API void recompileShaders();

	ENGINE_DLL_API void setBlendMode(BlendingConfig configuration);
	ENGINE_DLL_API void renderNavMesh(dtNavMesh const& navMesh);

	// HDR controls
	ENGINE_DLL_API void setHDRExposure(float exposure);
	ENGINE_DLL_API float getHDRExposure() const;

	// Tone mapping controls
	ENGINE_DLL_API void setToneMappingMethod(ToneMappingMethod method);
	ENGINE_DLL_API ToneMappingMethod getToneMappingMethod() const;

public:
	// =============================================
	// These interfaces are provided to the physics debug renderer for rendering debug colliders.
	// =============================================

	// submit triangles to be rendered at the end
	void submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3);
	void submitNavMeshTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3);

public:
	bool toGammaCorrect;

private:
	// =============================================
	// Private internal helper functions.
	// =============================================

	// set up proper configurations and clear framebuffers..
	void prepareRendering();

	// set up game object camera framebuffers
	void prepareGameViewport();

	// render skybox
	void renderSkyBox();

	// render all MeshRenderers.
	void renderModels();

	// renders a outline during object hovering and selection.
	void renderOutline();

	// Render all particles
	void renderParticles();

	// renders the object id to the object id framebuffer.
	void renderObjectId(GLsizei count);

	// render a debug triangles in physics
	void debugRenderPhysicsCollider();

	// render a debug triangles in navMesh
	void debugRenderNavMesh();

	// render debug shapes in particle emitter
	void debugRenderParticleEmissionShape();

	// HDR post-processing functions
	void renderHDRTonemapping();

	// the different rendering pipelines..
	// uses the corresponding shader, and sets up corresponding uniform based on rendering pipeline and material.
	void setupBlinnPhongShader(Material const& material);
	void setupPBRShader(Material const& material);
	void setupColorShader(Material const& material);

	// sets model specific uniforms for all rendering pipeline. (like model matrix)
	void setModelUniforms(Transform const& transform, entt::entity entity);

	// attempts to get the appropriate material from meshrenderer.
	Material const* obtainMaterial(MeshRenderer const& meshRenderer, Model::Mesh const& mesh);

	void printOpenGLDriverDetails() const;

	// =============================================
	// Main Frame Buffer operations..
	// =============================================
	FrameBuffer const& getActiveMainFrameBuffer() const;
	FrameBuffer const& getReadMainFrameBuffer() const;

	void swapMainFrameBuffers();

private:
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	// Main VAO and their related buffers
	GLuint mainVAO;
	BufferObject mainVBO;
	BufferObject EBO;

	// SSBO and UBO.
	BufferObject pointLightSSBO;
	BufferObject directionalLightSSBO;
	BufferObject spotLightSSBO;
	BufferObject sharedUBO;

	// Debug Physics VAO and it's corresponding VBO.
	GLuint debugVAO;
	BufferObject debugPhysicsVBO;
	BufferObject debugNavMeshVBO;
	BufferObject debugParticleShapeVBO;

	Camera camera;
	Camera gameCamera;

	// contains all the final rendering.
	// we use 2 frame buffers to alternate between the two between post processing..
	std::array<FrameBuffer, 2> mainFrameBuffers;

	int mainFrameBufferActiveIndex = 0;	// which framebuffer we are current writing to, and contains the latest image.
	int mainFrameBufferReadIndex  = 1;	// which framebuffer we are current reading from, to do additional post processing..

	// same thing but for game viewport camera
	std::array<FrameBuffer, 2> gameVPFrameBuffers;
	int gameVPFrameBuffersActiveIndex = 0;
	int gameVPFrameBuffersReadIndex = 1;

	// contains all physics debug rendering..
	FrameBuffer physicsDebugFrameBuffer;

	// contains objectIds for object picking.
	FrameBuffer objectIdFrameBuffer;

private:
	int numOfPhysicsDebugTriangles;
	int numOfNavMeshDebugTriangles;

	bool isOnWireframeMode;
	

public:
	Shader basicShader;
	Shader standardShader;
	Shader textureShader;
	Shader colorShader;
	Shader gridShader;
	Shader outlineShader;
	Shader blinnPhongShader;
	Shader PBRShader;
	Shader debugShader;
	Shader overlayShader;
	Shader objectIdShader;
	Shader skyboxShader;
	Shader particleShader;
	
	// HDR tone mapping shader
	Shader toneMappingShader;

	// HDR parameters
	float hdrExposure;
	ToneMappingMethod toneMappingMethod;
};
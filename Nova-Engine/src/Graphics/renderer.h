#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "bufferObject.h"
#include "framebuffer.h"
#include "pairFrameBuffer.h"
#include "ECS/ECS.h"
#include "component.h"
#include "vertex.h"
#include "font.h"

#include "model.h"
#include "cubemap.h"

#include "Detour/Detour/DetourNavMesh.h"
#include "customShader.h"

class Engine;
class ResourceManager;

class Renderer {
public:
	enum class ToneMappingMethod {
		Exposure,
		Reinhard,
		ACES,
		None
	};

	enum class MeshType {
		Normal,
		Skinned
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

	void renderMain(RenderConfig renderConfig);

	void render(PairFrameBuffer& frameBuffers, Camera const& camera);
	
	void renderToDefaultFBO();

public:
	// =============================================
	// Public facing API.
	// =============================================

	// get the main texture of the main frame buffer.
	ENGINE_DLL_API GLuint getEditorFrameBufferTexture() const;
	ENGINE_DLL_API GLuint getGameFrameBufferTexture() const;

	ENGINE_DLL_API void enableWireframeMode(bool toEnable);

	// gets object id from color attachment 1 of the main framebuffer.
	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	ENGINE_DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

	ENGINE_DLL_API Camera& getEditorCamera();
	ENGINE_DLL_API Camera const& getEditorCamera() const;

	ENGINE_DLL_API Camera& getGameCamera();
	ENGINE_DLL_API Camera const& getGameCamera() const;

	// most probably for ease of development.
	ENGINE_DLL_API void recompileShaders();

	ENGINE_DLL_API void setBlendMode(CustomShader::BlendingConfig configuration);
	ENGINE_DLL_API void setDepthMode(CustomShader::DepthTestingMethod configuration);

	ENGINE_DLL_API void renderNavMesh(dtNavMesh const& navMesh);
	ENGINE_DLL_API void renderObjectIds();

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

	// render skybox
	void renderSkyBox();

	// render all MeshRenderers.
	void renderModels(Camera const& camera);

	// render all SkinnedMeshRenderers.
	void renderSkinnedModels(Camera const& camera);

	// render all Texts.
	void renderTexts();

	// renders a outline during object hovering and selection.
	void renderOutline();

	// Render all particles
	void renderParticles();

	// render a debug triangles in physics
	void debugRenderPhysicsCollider();

	// render a debug triangles in navMesh
	void debugRenderNavMesh();

	// render debug shapes in particle emitter
	void debugRenderParticleEmissionShape();

	// HDR post-processing functions
	void renderHDRTonemapping(PairFrameBuffer& frameBuffers);

	// set up the material's chosen shader and supply the proper uniforms..
	// returns the material's underlying custom shader if setup is successful, otherwise nullptr.
	CustomShader* setupMaterial(Camera const& camera, Material const& material, Transform const& transform);

	// given a mesh and it's material, upload the necessary data to the VBOs and EBOs and issue a draw call.
	void renderMesh(Mesh const& mesh, Pipeline pipeline, MeshType meshType);

	Material const* obtainMaterial(MeshRenderer const& meshRenderer, Mesh const& mesh);
	Material const* obtainMaterial(SkinnedMeshRenderer const& skinnedMeshRenderer, Mesh const& mesh);

	void printOpenGLDriverDetails() const;

private:
	Engine& engine;
	ResourceManager& resourceManager;
	entt::registry& registry;

	// Main VAO and their related buffers
	GLuint mainVAO;
	BufferObject positionsVBO;				// VA 0
	BufferObject textureCoordinatesVBO;		// VA 1
	BufferObject normalsVBO;				// VA 2
	BufferObject tangentsVBO;				// VA 3

	// Skeletal animation VBO..
	BufferObject skeletalVBO;				// VA 4
	BufferObject EBO;						// VA 5

	// SSBO and UBO.
	BufferObject pointLightSSBO;
	BufferObject directionalLightSSBO;
	BufferObject spotLightSSBO;
	BufferObject sharedUBO;

	// Skeletal animation, bones SSBO
	BufferObject bonesSSBO;

	// Particle VAO and VBO
	GLuint particleVAO;
	BufferObject particleVBO;

	// Debug Physics VAO and it's corresponding VBO.
	BufferObject debugPhysicsVBO;
	BufferObject debugNavMeshVBO;
	BufferObject debugParticleShapeVBO;

	// Text VAO and VBO
	GLuint textVAO;
	BufferObject textVBO;

	Camera editorCamera;
	Camera gameCamera;

	PairFrameBuffer editorMainFrameBuffer;
	PairFrameBuffer gameMainFrameBuffer;

	// contains all physics debug rendering..
	FrameBuffer physicsDebugFrameBuffer;

	// contains objectIds for object picking.
	FrameBuffer objectIdFrameBuffer;
	
	glm::mat4 UIProjection;

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
	Shader skeletalAnimationShader;
	Shader textShader;

	// HDR tone mapping shader
	Shader toneMappingShader;

	// HDR parameters
	float hdrExposure;
	ToneMappingMethod toneMappingMethod;
};
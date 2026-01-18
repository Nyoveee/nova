#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>
#include <unordered_map>

#include "export.h"

#include "shader.h"
#include "computeShader.h"
#include "camera.h"
#include "bufferObject.h"
#include "lightSSBO.h"
#include "framebuffer.h"
#include "pairFrameBuffer.h"
#include "ECS/ECS.h"
#include "component.h"
#include "vertex.h"
#include "font.h"
#include "bloomFrameBuffer.h"
#include "depthFrameBuffer.h"
#include "texture2dArray.h"

#include "model.h"

#include "Detour/Detour/DetourNavMesh.h"

#include "materialConfig.h"

class Engine;
class ResourceManager;

struct MeshBOs {
	BufferObject positionsVBO{ BufferObject{0} };			// VA 0
	BufferObject textureCoordinatesVBO{ BufferObject{0} };	// VA 1
	BufferObject normalsVBO{ BufferObject{0} };				// VA 2
	BufferObject tangentsVBO{ BufferObject{0} };			// VA 3

	// Skeletal animation VBO..
	BufferObject skeletalVBO{ BufferObject{0} };			// VA 4
	BufferObject EBO{ BufferObject{0} };					// VA 5
};
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

	void renderUI();

	// renders the main scene in the perspective of given camera. light storage is provided by lightSSBO.
	void render(PairFrameBuffer& frameBuffers, Camera const& camera, LightSSBO& lightSSBO, BufferObject const& clusterSSBO);
	
	void renderToDefaultFBO();

	void renderBloom(PairFrameBuffer& frameBuffers);

	void overlayUIToBuffer(PairFrameBuffer& target);

	// generates shadow maps for each light source..
	void shadowPass(Camera const& camera);

	// does a depth pre pass and populates the gbuffer for ssao generation.
	void depthPrePass(Camera const& camera);

	// generates the SSAO texture.
	void generateSSAO(PairFrameBuffer& frameBuffers, Camera const& camera);

	// initialise the sample kernel and noise texture used in SSAO
	void initialiseSSAO();

public:
	// =============================================
	// Public facing API.
	// =============================================

	// get the main texture of the main frame buffer.
	ENGINE_DLL_API GLuint getEditorFrameBufferTexture() const;
	ENGINE_DLL_API GLuint getGameFrameBufferTexture() const;
	ENGINE_DLL_API GLuint getUIFrameBufferTexture() const;

	ENGINE_DLL_API GLuint getUBOId() const;

	ENGINE_DLL_API GLuint getEditorFrameBufferId() const;

	ENGINE_DLL_API void enableWireframeMode(bool toEnable);

	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	ENGINE_DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

	ENGINE_DLL_API GLuint getObjectUiId(glm::vec2 normalisedPosition) const;

	ENGINE_DLL_API Camera& getEditorCamera();
	ENGINE_DLL_API Camera const& getEditorCamera() const;

	ENGINE_DLL_API Camera& getGameCamera();
	ENGINE_DLL_API Camera const& getGameCamera() const;

	// most probably for ease of development.
	ENGINE_DLL_API void recompileShaders();

	ENGINE_DLL_API void setBlendMode(BlendingConfig configuration);
	ENGINE_DLL_API void setDepthMode(DepthTestingMethod configuration);
	ENGINE_DLL_API void setCullMode(CullingConfig configuration);

	ENGINE_DLL_API void renderNavMesh(dtNavMesh const& navMesh);
	ENGINE_DLL_API void renderObjectIds();
	ENGINE_DLL_API void renderUiObjectIds();

	// HDR controls
	ENGINE_DLL_API void setHDRExposure(float exposure);
	ENGINE_DLL_API float getHDRExposure() const;

	// Tone mapping controls
	ENGINE_DLL_API void setToneMappingMethod(ToneMappingMethod method);
	ENGINE_DLL_API ToneMappingMethod getToneMappingMethod() const;

	// UI projection
	ENGINE_DLL_API const glm::mat4& getUIProjection() const;
	ENGINE_DLL_API void randomiseChromaticAberrationoffset();

public:
	// Editor rendering..
	ENGINE_DLL_API void submitSelectedObjects(std::vector<entt::entity> const& entities);
	ENGINE_DLL_API void renderDebugSelectedObjects();


	// first renders the scene with the given render function onto an intermediate framebuffer, then
	// bakes it into a convoluted diffuse irradiance map.
	// the scene is rendered 6 times.
	ENGINE_DLL_API CubeMap bakeDiffuseIrradianceMap(std::function<void()> render);

	// for specular irradiance map..
	ENGINE_DLL_API CubeMap bakeSpecularIrradianceMap(std::function<void()> render);

	// render first skybox in the scene, if any.
	ENGINE_DLL_API void renderSkyBox();

	// renders skybox given an equirectangular map.
	ENGINE_DLL_API void renderSkyBox(EquirectangularMap const& equirectangularMap);

	// renders skybox given an cubemap.
	ENGINE_DLL_API void renderSkyBox(CubeMap const& cubemap);

public:
	// =============================================
	// These interfaces are provided to the physics debug renderer for rendering debug colliders.
	// =============================================

	// submit triangles to be rendered at the end
	void submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3);
	void submitLine(glm::vec3 vertice1, glm::vec3 vertice2);

	void submitNavMeshTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3);

public:
	bool isEditorScreenShown;
	bool isGameScreenShown;
	bool isUIScreenShown;
	bool toGammaCorrect;
	bool toPostProcess;

	float bloomFilterRadius = 0.005f;
	float bloomCompositePercentage = 0.04f;

	float vignette = 0.f;

	glm::vec3 chromaticAberration;

private:
	// =============================================
	// Private internal helper functions.
	// =============================================

	// set up proper configurations and clear framebuffers..
	void prepareRendering();

	// render all MeshRenderers.
	void renderModels(Camera const& camera, bool normalOnly = false);

	// render all TranslucentMeshRenderers.
	void renderTranslucentModels(Camera const& camera);

	// render all SkinnedMeshRenderers.
	void renderSkinnedModels(Camera const& camera, bool normalOnly = false);

	// render all Texts.
	void renderText(Transform const& transform, Text const& text);

	// render ui images.
	void renderImage(Transform const& transform, Image const& image, ColorA const& colorMultiplier);

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

	// renders the bounding volume to debug frustum culling..
	void debugRenderBoundingVolume();

	// renders all the clusters of the camera..
	void debugRenderClusters();

	// main debug render function
	void debugRender();

	// Calls the relevant compute shader Pre Post Process
	void computePostProcessing(PairFrameBuffer& frameBuffers, Camera const& camera);

	// renders post processing effect on the pair framebuffer
	void renderPostProcessing(PairFrameBuffer& frameBuffers);

	// HDR post-processing functions
	void renderHDRTonemapping(PairFrameBuffer& frameBuffers);

	// set up the material's chosen shader and supply the proper uniforms..
	// returns the material's underlying custom shader if setup is successful, otherwise nullptr.
	CustomShader* setupMaterial(Camera const& camera, Material const& material, Transform const& transform, float scale = 1.f);

	// sets up the custom shader to output the mesh into the normal buffer instead.
	CustomShader* setupMaterialNormalPass(Material const& material, Transform const& transform, float scale = 1.f);

	// void set up all the uniforms for the custom shader.
	void setupCustomShaderUniforms(Shader const& shader, Material const& material, int numOfTextureUnitsUsed = 0);

	// given a mesh and it's material, upload the necessary data to the VBOs and EBOs and issue a draw call.
	void renderMesh(Mesh& mesh, Pipeline pipeline, MeshType meshType);

	// helper function to obtain the underlying material of a mesh given its renderers.
	Material const* obtainMaterial(MeshRenderer const& meshRenderer, Mesh const& mesh);
	Material const* obtainMaterial(TranslucentMeshRenderer const& transMeshRenderer, Mesh const& mesh);
	Material const* obtainMaterial(SkinnedMeshRenderer const& skinnedMeshRenderer, Mesh const& mesh);

	// performs frustum culling for models and light
	void frustumCullModels(glm::mat4 const& viewProjectionMatrix);
	void frustumCullLight(glm::mat4 const& viewProjectionMatrix);

	// upload lights into SSBO
	void prepareLights(Camera const& camera, LightSSBO& lightSBBO);

	// builds clusters information for clustered forward rendering..
	void clusterBuilding(Camera const& camera, BufferObject const& clusterSSBO);

	// Calculates the camera's frustum.
	Frustum calculateCameraFrustum(glm::mat4 const& viewProjectionMatrix);

	// Calculate the AABB bounding box of a given model.
	AABB calculateAABB(Model const& model, Transform const& transform);

	void printOpenGLDriverDetails() const;

	// populates the directional light shadow pass
	void shadowPassRender(glm::mat4 const& viewProjectionMatrix);

	// set up the required uniforms for normal map
	void setupNormalMapUniforms(Shader& shader, Material const& material);

	// construct all required VBOs and EBO of the meshes if not constructed.
	void constructMeshBuffers(Mesh& mesh);

	// swap buffers to a new mesh to its respective buffer binding index
	void swapVertexBuffer(Mesh& mesh);

	// updates the camera UBO with camera information.
	void updateCameraUBO(Camera const& camera);

	// captures surrounding for baking irradiance maps..
	CubeMap captureSurrounding(std::function<void()> render);

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

	// List of mesh VBOs
	std::unordered_map<MeshID, MeshBOs> meshBOs; // VA 0 - 4

	// SSBO and UBO.
	LightSSBO gameLights;
	LightSSBO editorLights;
	
	BufferObject gameClusterSSBO;
	BufferObject editorClusterSSBO;

	BufferObject cameraUBO;
	BufferObject PBRUBO;

	// Skeletal animation, bones SSBO
	BufferObject bonesSSBO;

	// Volumetric Fog SSBO
	BufferObject volumetricFogSSBO;

	// stores the shadow caster matrixes in a UBO.
	BufferObject shadowCasterMatrixes;

	// Particle VAO and VBO
	GLuint particleVAO;

	// Debug Physics VAO and it's corresponding VBO.
	BufferObject debugPhysicsVBO;
	BufferObject debugPhysicsLineVBO;

	BufferObject debugNavMeshVBO;
	BufferObject debugParticleShapeVBO;

	// Text VAO and VBO
	GLuint textVAO;
	BufferObject textVBO;

	GLuint ssaoNoiseTextureId;

	Camera editorCamera;
	Camera gameCamera;

	PairFrameBuffer editorMainFrameBuffer;
	PairFrameBuffer gameMainFrameBuffer;

	PairFrameBuffer ssaoFrameBuffer;
	BloomFrameBuffer bloomFrameBuffer;

	// shadow map..
	DepthFrameBuffer directionalLightShadowFBO;

	DepthFrameBuffer shadowFBO; // smaller than directionalLight
	Texture2DArray spotlightShadowMaps;

	// contains all physics debug rendering..
	FrameBuffer uiMainFrameBuffer;
	FrameBuffer physicsDebugFrameBuffer;

	// contains objectIds for object picking.
	FrameBuffer objectIdFrameBuffer;
	FrameBuffer uiObjectIdFrameBuffer;

	// contains an intermediary framebuffer for baking irradiance map
	FrameBuffer cubeMapFrameBuffer;
	FrameBuffer diffuseIrradianceMapFrameBuffer;

	glm::mat4 UIProjection;

	// BRDF Look Up Table, used for split sum approximation for specular IBL.
	std::unique_ptr<Texture> BRDFLUT;

private:
	unsigned int numOfPtLights;

	int numOfPhysicsDebugTriangles;
	int numOfPhysicsDebugLines;
	int numOfNavMeshDebugTriangles;

	int gameWidth;
	int gameHeight;

	glm::vec2 gameSize { gameWidth, gameHeight };

	bool isOnWireframeMode;

	float timeElapsed;
	std::vector<entt::entity> selectedEntities;

	bool hasDirectionalLightShadowCaster;
	glm::mat4x4 directionalLightViewMatrix;
	glm::vec3 directionalLightDir;
	
	int numOfSpotlightShadowCaster;

public:
	Shader basicShader;
	Shader standardShader;
	Shader textureShader;
	Shader colorShader;
	Shader gridShader;
	Shader outlineShader;
	Shader debugShader;
	Shader overlayShader;
	
	Shader objectIdShader;
	Shader uiImageObjectIdShader;
	Shader uiTextObjectIdShader;
	
	Shader skyboxShader;
	Shader skyboxCubemapShader;
	Shader particleShader;
	Shader textShader;
	Shader texture2dShader;

	// HDR tone mapping shader
	Shader toneMappingShader;

	Shader bloomDownSampleShader;
	Shader bloomUpSampleShader;
	Shader bloomFinalShader;

	Shader postprocessingShader;

	Shader shadowMapShader;
	// Shader depthGBufferShader;
	Shader ssaoShader;
	Shader gaussianBlurShader;

	Shader bakeDiffuseIrradianceMapShader;	
	Shader bakeSpecularIrradianceMapShader;

	// Compute shaders..
	ComputeShader clusterBuildingCompute;
	ComputeShader clusterLightCompute;
	ComputeShader rayMarchingVolumetricFogCompute;
	ComputeShader volumetricFogBufferResetCompute;

	// HDR parameters
	float hdrExposure;
	ToneMappingMethod toneMappingMethod;

	// Used to debug frustum culling..
	bool toDebugRenderBoundingVolume = false;
	bool toDebugClusters = false;

	bool toEnableSSAO = true;
};
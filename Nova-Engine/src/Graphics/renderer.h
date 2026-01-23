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
#include "cubemapArray.h"

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

enum class RenderMode {
	Editor,
	Game
};

class Renderer {
public:
	enum class MeshType {
		Normal,
		Skinned
	};

public:
	Renderer(Engine& engine, RenderConfig renderConfig, int gameWidth, int gameHeight);

	~Renderer();
	Renderer(Renderer const& other)				= delete;
	Renderer(Renderer&& other)					= delete;
	Renderer& operator=(Renderer const& other)	= delete;
	Renderer& operator=(Renderer&& other)		= delete;

public:
	void update(float dt);

	void renderMain(RenderMode renderMode);

	void renderUI();

	// renders the main scene in the perspective of given camera. light storage is provided by lightSSBO.
	void render(PairFrameBuffer& frameBuffers, Camera const& camera);
	
	void renderToDefaultFBO();

	void renderBloom(PairFrameBuffer& frameBuffers);

	void overlayUIToBuffer(PairFrameBuffer& target);

	// generates shadow maps for each light source..
	void shadowPass(int viewportWidth, int viewportHeight);

	// does a depth pre pass and populates the gbuffer for ssao generation.
	void depthPrePass(Camera const& camera);

	// generates the SSAO texture.
	void generateSSAO(PairFrameBuffer& frameBuffers, Camera const& camera);

	// initialise the sample kernel and noise texture used in SSAO
	void initialiseSSAO();	

	// when changing scene, mark all reflection probes as unloaded.
	void resetLoadedReflectionProbes();

	void handleReflectionProbeDeletion(entt::registry&, entt::entity entityID);

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

	ENGINE_DLL_API CubeMap bakeDiffuseIrradianceMap(ReflectionProbe const& reflectionProbe, glm::vec3 const& position, bool toCaptureEnvironmentLight);
	ENGINE_DLL_API CubeMap bakePrefilteredEnvironmentMap(ReflectionProbe const& reflectionProbe, glm::vec3 const& position, bool toCaptureEnvironmentLight);

	// render first skybox in the scene, if any.
	ENGINE_DLL_API void renderSkyBox();

	// renders skybox given an equirectangular map.
	ENGINE_DLL_API void renderSkyBox(EquirectangularMap const& equirectangularMap);

	// renders skybox given an cubemap.
	ENGINE_DLL_API void renderSkyBox(CubeMap const& cubemap);

	// load a reflection probe's cube map to cube maparray if not loaded..
	// this function assumes a valid index has already been assigned to reflection probe.
	ENGINE_DLL_API void loadReflectionProbe(ReflectionProbe const& reflectionProbe, CubeMap const& reflectionProbePrefilteredMap);

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

	// renders object for the purpose of capturing into a cubemap..
	void renderCapturePass(Camera const& camera, std::function<void()> setupFramebuffer, bool toCaptureEnvironmentLight);

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
	void computeFog(PairFrameBuffer& frameBuffers, Fog const& fog);

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
	void prepareLights();

	// upload reflection probes into UBO
	// pass in disable to set reflection probe to 0.
	void prepareReflectionProbes(Camera const& camera);

	// builds clusters information for clustered forward rendering..
	void clusterBuilding(Camera const& camera);

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

	// captures surrounding via the POV of reflection probe.
	CubeMap captureSurrounding(ReflectionProbe const& reflectionProbe, glm::vec3 const& position, bool toCaptureEnvironmentLight);

	// Convolutes the respective IBL maps..
	CubeMap convoluteIrradianceMap(CubeMap const& inputCubemap);
	CubeMap convolutePrefilteredEnvironmentMap(CubeMap const& inputCubemap);

	// finds a suitable index to assign a reflection probe to the cubemap array.
	// will return a number greater than MAX_REFLECTION_PROBES if no more slot. (rare imo)
	int getIndexToCubeMapArray();

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
	LightSSBO lightSSBO;				// SSBO 0 - 2 (Pointlight, Spotlight, DirectionalLight)	
	BufferObject bonesSSBO;				// SSBO 3, bones SSBO
	BufferObject clusterSSBO;			// SSBO 7
	BufferObject volumetricFogSSBO;		// SSBO 8, Volumetric Fog SSBO

	BufferObject cameraUBO;				// UBO 0
	BufferObject shadowCasterMatrixes;	// UBO 1 stores the shadow caster matrixes in a UBO.
	BufferObject PBRUBO;				// UBO 2
	BufferObject reflectionProbesUBO;	// UBO 3

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
	Camera bakingCamera;

	PairFrameBuffer editorMainFrameBuffer;
	PairFrameBuffer gameMainFrameBuffer;

	PairFrameBuffer ssaoFrameBuffer;
	BloomFrameBuffer bloomFrameBuffer;

	// shadow map..
	DepthFrameBuffer directionalLightShadowFBO;

	DepthFrameBuffer shadowFBO; // smaller than directionalLight
	Texture2DArray spotlightShadowMaps;

	CubeMapArray loadedReflectionProbesMap;

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

	// when a reflection probe is deleted, the index is now free to use.
	std::unordered_set<int> freeCubeMapArraySlots;

private:

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
	int numOfLoadedReflectionProbe;

public:
	RenderConfig renderConfig;

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

	// HDR parameters
	float hdrExposure;

	// Used to debug frustum culling..
	bool toDebugRenderBoundingVolume = false;
	bool toDebugClusters = false;
};
#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "bufferObject.h"
#include "framebuffer.h"
#include "Component/ECS.h"

#include "model.h"
#include "cubemap.h"

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

	enum class RenderTarget {
		ToDefaultFrameBuffer,
		ToMainFrameBuffer
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

	// choose to either render to the default frame buffer or the main frame buffer.
	void render(RenderTarget target, bool toRenderDebug);

public:
	// =============================================
	// Public facing API.
	// =============================================

	// used directly in the editor. i need to export this.
	DLL_API std::vector<GLuint> const& getMainFrameBufferTextures() const;
	DLL_API void enableWireframeMode(bool toEnable);

	// gets object id from color attachment 1 of the main framebuffer.
	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

	DLL_API Camera& getCamera();
	DLL_API Camera const& getCamera() const;

	// most probably for ease of development.
	DLL_API void recompileShaders();

public:
	// =============================================
	// These interfaces are provided to the physics debug renderer for rendering debug colliders.
	// =============================================

	// submit triangles to be rendered at the end
	void submitTriangle(glm::vec3 vertice1, glm::vec3 vertice2, glm::vec3 vertice3, ColorA color);

private:
	// =============================================
	// Private internal helper functions.
	// =============================================

	// set up proper configurations and clear framebuffers..
	void prepareRendering(RenderTarget target);

	// render skybox
	void renderSkyBox();

	// render all MeshRenderers.
	void renderModels();

	// renders a outline during object hovering and selection.
	void renderOutline();

	// renders the object id to the object id framebuffer.
	void renderObjectId(GLsizei count);

	// render a debug triangles in physics
	void debugRender();

	// the different rendering pipelines..
	// uses the corresponding shader, and sets up corresponding uniform based on rendering pipeline and material.
	void setupBlinnPhongShader(Material const& material);
	void setupPBRShader(Material const& material);
	void setupColorShader(Material const& material);

	// sets model specific uniforms for all rendering pipeline. (like model matrix)
	void setModelUniforms(Transform const& transform, entt::entity entity);

	// attempts to get the appropriate material from meshrenderer.
	Material const* obtainMaterial(MeshRenderer const& meshRenderer, Model::Mesh const& mesh);

	void setBlendMode(BlendingConfig configuration);
	void printOpenGLDriverDetails() const;

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
	GLuint debugPhysicsVAO;
	BufferObject debugPhysicsVBO;

	Camera camera;

	// contains all the final rendering.
	FrameBuffer mainFrameBuffer;

	// contains all physics debug rendering..
	FrameBuffer physicsDebugFrameBuffer;

	// contains objectIds for object picking.
	FrameBuffer objectIdFrameBuffer;

private:
	int numOfDebugTriangles;
	bool isOnWireframeMode;
	unsigned int drawcallCounter;

public:
	// get the drawcall
	DLL_API unsigned int drawCalls();
public:
	Shader basicShader;
	Shader standardShader;
	Shader textureShader;
	Shader colorShader;
	Shader gridShader;
	Shader outlineShader;
	Shader blinnPhongShader;
	Shader debugShader;
	Shader debugOverlayShader;
	Shader objectIdShader;
	Shader skyboxShader;
};
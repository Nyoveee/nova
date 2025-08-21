#pragma once

#include <glad/glad.h>
#include <vector>
#include <entt/entt.hpp>

#include "export.h"

#include "shader.h"
#include "camera.h"
#include "bufferObject.h"
#include "framebuffer.h"
#include "ECS.h"

#include "AssetManager/Asset/model.h"

class Engine;
class AssetManager;

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
	// used directly in the editor. i need to export this.
	
	DLL_API std::vector<GLuint> const& getMainFrameBufferTextures() const;
	DLL_API void enableWireframeMode(bool toEnable) const;

	// gets object id from color attachment 1 of the main framebuffer.
	// parameter normalisedPosition expects value of range [0, 1], representing the spot in the color attachment from bottom left.
	// retrieves the value in that position of the framebuffer.
	DLL_API GLuint getObjectId(glm::vec2 normalisedPosition) const;

	DLL_API Camera& getCamera();
	DLL_API Camera const& getCamera() const;

	// most probably for ease of development.
	DLL_API void recompileShaders();

public:
	void update(float dt);

	// choose to either render to the default frame buffer or the main frame buffer.
	void render(RenderTarget target);

private:
	// set up proper configurations and clear framebuffers..
	void prepareRendering(RenderTarget target);

	// render all MeshRenderers.
	void renderModels();

	// renders a outline during object hovering and selection.
	void renderOutline();

	// the different rendering pipelines..
	// uses the corresponding shader, and sets up corresponding uniform based on rendering pipeline and material.
	void setupBlinnPhongShader(Material const& material);
	void setupPBRShader(Material const& material);
	void setupColorShader(Material const& material);

	// sets model specific uniforms for all rendering pipeline. (like model matrix)
	void setModelUniforms(Transform const& transform, entt::entity entity);

	// attempts to get the appropriate material from meshrenderer.
	Material const* obtainMaterial(MeshRenderer const& meshRenderer, Model::Mesh const& mesh);

private:

	void setBlendMode(BlendingConfig configuration);
	void printOpenGLDriverDetails() const;

private:
	Engine& engine;
	AssetManager& assetManager;
	entt::registry& registry;

	BufferObject VBO;
	BufferObject EBO;
	GLuint VAO;

	Camera camera;

	// contains all the final rendering.
	FrameBuffer mainFrameBuffer;

public:
	Shader basicShader;
	Shader standardShader;
	Shader textureShader;
	Shader colorShader;
	Shader gridShader;
	Shader outlineShader;
	Shader blinnPhongShader;
};
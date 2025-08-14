#include <iostream>

#include "engine.h"
#include "window.h"

#include "Component/component.h"
#include "inputManager.h"

#include <GLFW/glfw3.h>

Engine::Engine(Window& window, InputManager& inputManager, AssetManager& assetManager, int gameWidth, int gameHeight) :
	window			{ window },
	assetManager	{ assetManager },
	renderer		{ *this, gameWidth, gameHeight },
	cameraSystem	{ *this, inputManager },
	ecs				{},
	gameWidth		{ gameWidth },
	gameHeight		{ gameHeight }
{}

void Engine::fixedUpdate(float dt) {
	(void) dt;
}

void Engine::update(float dt) {
	static float zPos = 0.f;

	if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_0)) {
		auto entity = ecs.registry.create();

		zPos -= 2.f;

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		ecs.registry.emplace<Transform>(entity, std::move(transform));

		std::unordered_map<MaterialName, MeshRenderer::Material> materials;

		AssetID modelAsset{ 1 };
		materials["Table_frame_mtl"] = { AssetID{3} };
		materials["Table_top_mtl"] = { AssetID{4} };

		ecs.registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });
	}

	cameraSystem.update(dt);
	renderer.update(dt);
}

void Engine::render(Renderer::RenderTarget target) {
	renderer.render(target);
}

int Engine::getGameWidth() const {
	return gameWidth;
}

int Engine::getGameHeight() const {
	return gameHeight;
}

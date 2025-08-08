#include <iostream>

#include "../Header/engine.h"
#include "../Header/window.h"
#include "../../Graphics/renderer.h"
#include "../../Graphics/cameraSystem.h"
#include "../../ECS & Components/ECS.h"
#include "../../ECS & Components/component.h"

#include <GLFW/glfw3.h>

Engine::Engine(Window& window) :
	window			{ window },
	renderer		{ Renderer::instance() },
	cameraSystem	{ CameraSystem::instance() },
	ecs				{ ECS::instance() }
{}

void Engine::fixedUpdate(float dt) {
	(void) dt;
}

void Engine::update(float dt) {
	entt::registry& registry = ecs.registry;

	if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_0) == GLFW_PRESS) {
		auto entity = registry.create();

		static float zPos = 0;
		zPos -= 5.f;

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		Mesh mesh = {
			{
				Vertex{{  0.5f,  0.5f,  0.f }, { 1.0f, 1.0f }},	// top right
				Vertex{{ -0.5f, -0.5f,  0.f }, { 0.0f, 0.0f }},	// bottom left
				Vertex{{  0.5f, -0.5f,  0.f }, { 1.0f, 0.0f }},	// bottom right

				Vertex{{ -0.5f,  0.5f,  0.f }, { 0.0f, 1.0f }},	// top left
				Vertex{{ -0.5f, -0.5f,  0.f }, { 0.0f, 0.0f }},	// bottom left
				Vertex{{  0.5f,  0.5f,  0.f }, { 1.0f, 1.0f }},	// top right
			},
			Color{}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<Mesh>(entity, std::move(mesh));
	}

	cameraSystem.update();
	renderer.update(dt);
	renderer.render();
}

#include "../Header/window.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "editor.h"
#include "engine.h"
#include "ECS.h"
#include "component.h"
#include "themes.h"
#include "inputManager.h"

#include <GLFW/glfw3.h>

Editor::Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager) :
	window				{ window },
	engine				{ engine },
	assetManager		{ assetManager },
	gameViewPort		{ engine },
	inputManager		{ inputManager },
	componentInspector	{},
	assetManagerUi		{},
	hasEditorControl	{ false }
{
	(void) inputManager;

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup platform/renderer bindings (example with GLFW and OpenGL)
	ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), true);		// window is your GLFW window
	ImGui_ImplOpenGL3_Init("#version 450");							// or appropriate GLSL version

	ImGuiTheme::Dark();

	// Subscribe to input manager for editor control.
	inputManager.subscribe<ToggleEditorControl>(
		[&](ToggleEditorControl) {
			if(gameViewPort.isHoveringOver) toggleEditorControl(true);
		},
		[&](ToggleEditorControl) {
			toggleEditorControl(false);
		}
	);
}

void Editor::update() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	main();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::main() {
	ImGui::ShowDemoWindow();

	entt::registry& registry = engine.ecs.registry;
	
	// Show all game objects..
	ImGui::Begin("Game objects");

	// ========================================
	// STUB CODE!!!!
	// ========================================

	ImGui::SliderFloat("Camera Speed", &engine.cameraSystem.cameraSpeed, 0.f, 10.f);

	if (ImGui::Button("(+) Add entity")) {
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
	
	for (auto&& [entity, transform, mesh] : registry.view<Transform, Mesh>().each()) {
		std::string textToDisplay =
			"Entity ID: " + std::to_string(static_cast<entt::id_type>(entity))
			+ ", pos: {"
			+ std::to_string(transform.position.x) + " "
			+ std::to_string(transform.position.y) + " "
			+ std::to_string(transform.position.z) + "} ";
		
		ImGui::Text(textToDisplay.c_str());
	}

	// ========================================
	// END OF STUB CODE
	// ========================================

	gameViewPort.update();
	componentInspector.update();
	assetManagerUi.update();

	ImGui::End();
}

void Editor::toggleEditorControl(bool toControl) {
	hasEditorControl = toControl;

	if (hasEditorControl) {
		inputManager.broadcast<ToCameraControl>(ToCameraControl::Control);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Disable);
		
	}
	else {
		inputManager.broadcast<ToCameraControl>(ToCameraControl::Release);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Enable);
	}
}

Editor::~Editor() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

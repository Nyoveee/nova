#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "window.h"
#include "editor.h"
#include "engine.h"
#include "ECS.h"
#include "themes.h"
#include "ScriptingAPIManager.h"
#include "inputManager.h"
#include "assetManager.h"

#include "AssetManager/Asset/model.h"

#include "Component/component.h"
#include <GLFW/glfw3.h>


Editor::Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager) :
	window					{ window },
	engine					{ engine },
	assetManager			{ assetManager },
	gameViewPort			{ engine },
	inputManager			{ inputManager },
	componentInspector		{},
	assetManagerUi			{},
	isControllingInViewPort	{ false }
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
			if (gameViewPort.isHoveringOver) {
				toggleViewPortControl(true);
				ImGui::GetIO().WantCaptureMouse = false;
			}
		},
		[&](ToggleEditorControl) {
			toggleViewPortControl(false);
			ImGui::GetIO().WantCaptureMouse = true;
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
	
	updateMaterialMapping();
	entt::registry& registry = engine.ecs.registry;
	
	// Show all game objects..
	ImGui::Begin("Game objects");

	// ========================================
	// STUB CODE!!!!
	// ========================================

	ImGui::SliderFloat("Camera Speed", &engine.cameraSystem.cameraSpeed, 0.f, 10.f);

#if 0
	if (ImGui::Button("(+) Add plane")) {
		auto entity = registry.create();
		static float zPos = 0;
		zPos -= 5.f;

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		std::vector<unsigned int> indices = {  // note that we start from 0!
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};

		ModelAsset::Mesh mesh = {
			{
				Vertex{{  0.5f,  0.5f,  0.f }, { 1.0f, 1.0f }},	// top right
				Vertex{{  0.5f, -0.5f,  0.f }, { 1.0f, 0.0f }},	// bottom right
				Vertex{{ -0.5f, -0.5f,  0.f }, { 0.0f, 0.0f }},	// bottom left
				Vertex{{ -0.5f,  0.5f,  0.f }, { 0.0f, 1.0f }},	// top left
			},
			std::move(indices),
			2
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<Mesh>(entity, std::move(mesh));
		registry.emplace<ModelRenderer>(entity, std::move(ModelRenderer{2}));
	}	
#endif

	static float zPos = 0;
	static bool alternate = true;

	if (ImGui::Button("(+) Add 3D model")) {
		auto entity = registry.create();

		zPos -= 2.f;

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		registry.emplace<Transform>(entity, std::move(transform));

		std::unordered_map<MaterialName, MeshRenderer::Material> materials;

		AssetID modelAsset{ 1 };

		if (alternate) {
			materials["Table_frame_mtl"] = { AssetID{3} };
			materials["Table_top_mtl"] = { AssetID{4} };
		}
		else {
			materials["Material_50"] = { AssetID{0} };
		}
		
		registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });
		
		alternate = !alternate;

		// Test
		//engine.scriptingAPIManager.loadScriptIntoAPI(static_cast<unsigned int>(entity), "TestScript");
	}

	for (auto&& [entity, transform, modelRenderer] : registry.view<Transform, MeshRenderer>().each()) {
		std::string textToDisplay =
			"Entity ID: " + std::to_string(static_cast<entt::id_type>(entity))
			+ ", pos: {"
			+ std::to_string(transform.position.x) + " "
			+ std::to_string(transform.position.y) + " "
			+ std::to_string(transform.position.z) + "} "
			+ ", asset id: " + std::to_string((std::size_t)modelRenderer.modelId);

		ImGui::Text(textToDisplay.c_str());
		
		auto [model, _] = assetManager.getAsset<Model>(modelRenderer.modelId);
		
		if (!model) continue;

		ImGui::Text("Materials:");
		for (auto const& materialName : model->materialNames) {
			ImGui::Text(std::string{ " 	-" + materialName }.c_str());
		}
	}

	// ========================================
	// END OF STUB CODE
	// ========================================

	gameViewPort.update();
	componentInspector.update();
	assetManagerUi.update();

	ImGui::End();
}

void Editor::toggleViewPortControl(bool toControl) {
	isControllingInViewPort = toControl;

	if (isControllingInViewPort) {
		inputManager.broadcast<ToCameraControl>(ToCameraControl::Control);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Disable);
		
	}
	else {
		inputManager.broadcast<ToCameraControl>(ToCameraControl::Release);
		inputManager.broadcast<ToEnableCursor>(ToEnableCursor::Enable);
	}
}

void Editor::updateMaterialMapping() {
	entt::registry& registry = engine.ecs.registry;

	// Find all material names with the associated asset.
	for (auto&& [entity, modelRenderer] : registry.view<MeshRenderer>().each()) {
		auto [model, _] = assetManager.getAsset<Model>(modelRenderer.modelId);

		if (!model) {
			continue;
		}

		bool materialHasChanged = false;

		// check if there is a match. if there is a mismatch, the material requirement
		// of the mesh probably changed / changed mesh.
		for (auto const& materialName : model->materialNames) {
			auto iterator = modelRenderer.materials.find(materialName);

			if (iterator == modelRenderer.materials.end()) {
				materialHasChanged = true;
				break;
			}
		}

		if (!materialHasChanged) {
			return;
		}

		// lets reupdate our map.
		modelRenderer.materials.clear();

		for (auto const& materialName : model->materialNames) {
			modelRenderer.materials[materialName];
		}
	}
}

Editor::~Editor() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

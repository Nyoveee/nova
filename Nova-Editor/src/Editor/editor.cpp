#include "ScriptingAPIManager.h"

#include <glad/glad.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "window.h"
#include "editor.h"
#include "engine.h"
#include "ECS.h"
#include "themes.h"
#include "inputManager.h"
#include "assetManager.h"

#include "AssetManager/Asset/model.h"

#include "Component/component.h"

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
	
	handleEntitySelection();
	updateMaterialMapping();

	entt::registry& registry = engine.ecs.registry;
	
	// Show all game objects..
	ImGui::Begin("Game objects");

	// ========================================
	// STUB CODE!!!!
	// ========================================

	static bool wireFrameMode = false;

	if (ImGui::Button("Wireframe mode.")) {
		wireFrameMode = !wireFrameMode;
		engine.renderer.enableWireframeMode(wireFrameMode);
	}

	ImGui::Text("Camera Speed: %.2f", engine.cameraSystem.getCameraSpeed());

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
	}

	if (ImGui::Button("test script")) {
		entt::entity testSubject{ 0 };

		if (registry.valid(testSubject)) {
			engine.scriptingAPIManager.loadScriptIntoAPI((unsigned int)testSubject, "TestScript");
		}
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

void Editor::handleEntitySelection() {
	ImGui::Begin("text");
	ImGui::Text("%u", hoveringEntity);
	ImGui::End();
	if (	
			gameViewPort.mouseRelativeToViewPort.x < 0.f 
		||	gameViewPort.mouseRelativeToViewPort.x >= 1.f
		||	gameViewPort.mouseRelativeToViewPort.y < 0.f
		||	gameViewPort.mouseRelativeToViewPort.y >= 1.f
	) {
		return;
	}

	entt::entity newHoveringEntity =
		static_cast<entt::entity>(engine.renderer.getObjectId({ gameViewPort.mouseRelativeToViewPort.x, gameViewPort.mouseRelativeToViewPort.y }));

	if (newHoveringEntity == hoveringEntity) {
		return;
	}
	
	// new entity hovered.
	entt::registry& registry = engine.ecs.registry;
	
	// has component mesh renderer.
	if (registry.all_of<MeshRenderer>(newHoveringEntity)) {
		MeshRenderer& meshRenderer = registry.get<MeshRenderer>(newHoveringEntity);
		meshRenderer.toRenderOutline = true;
	}

	if (registry.all_of<MeshRenderer>(hoveringEntity)) {
		MeshRenderer& meshRenderer = registry.get<MeshRenderer>(hoveringEntity);
		meshRenderer.toRenderOutline = false;
	}

	hoveringEntity = newHoveringEntity;
}

Editor::~Editor() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

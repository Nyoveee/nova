#include "ScriptingAPIManager.h"

#include <glad/glad.h>

#include "IconsFontAwesome6.h"

#include "imgui.h"
#include "ImGuizmo.h"
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

#include <GLFW/glfw3.h>
#include <ranges>

constexpr float baseFontSize = 15.0f;
constexpr const char* fontFileName = 
	"System\\Font\\"
	"NotoSans-Medium.ttf";

Editor::Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager) :
	window					{ window },
	engine					{ engine },
	assetManager			{ assetManager },
	gameViewPort			{ *this },
	inputManager			{ inputManager },
	componentInspector		{ *this, engine.ecs },
	assetManagerUi			{},
	hierarchyList			{ engine.ecs, *this },
	isControllingInViewPort	{ false },
	hoveringEntity			{ entt::null }
{
	(void) inputManager;

	// ======================================= 
	// Preparing some ImGui config..
	// ======================================= 

	ImGui::CreateContext();
	ImGuiTheme::Dark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// ======================================= 
	// Prepare Font Awesome and custom font.
	// ======================================= 
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
	
	ImFont* myFont = io.Fonts->AddFontFromFileTTF(fontFileName, baseFontSize);
	io.FontDefault = myFont;

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF("System\\Font\\" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);

	// ======================================= 
	// Provide ImGui with our backend (GLFW + OpenGL)
	// ======================================= 
	
	ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), true);		// window is your GLFW window
	ImGui_ImplOpenGL3_Init("#version 450");							// or appropriate GLSL version

	// ======================================= 
	// 	Subscribe to input manager for editor control.
	// ======================================= 
		
	inputManager.subscribe<ToggleEditorControl>(
		[&](ToggleEditorControl) {
			if (gameViewPort.isHoveringOver) {
				toggleViewPortControl(true);
				ImGui::SetWindowFocus(nullptr);
			}
		},
		[&](ToggleEditorControl) {
			toggleViewPortControl(false);
		}
	);

	inputManager.subscribe<ToSelectGameObject>(
		[&](ToSelectGameObject) {
			handleEntitySelection();
		}
	);

	inputManager.subscribe<DeleteSelectedEntity>(
		[&](DeleteSelectedEntity) {
			// @TODO: Confirmation prompt LMAOO
			for (entt::entity entity : selectedEntities) {
				deleteEntity(entity);
			}
		}
	);
}

void Editor::update() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	main();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Editor::isEntitySelected(entt::entity entity) {
	return std::ranges::find(selectedEntities, entity) != std::end(selectedEntities);
}

bool Editor::hasAnyEntitySelected() const {
	return selectedEntities.size();
}

void Editor::selectEntities(std::vector<entt::entity> entities) {
	toOutline(selectedEntities, false);
	selectedEntities = std::move(entities);
	toOutline(selectedEntities, true);
}

std::vector<entt::entity> const& Editor::getSelectedEntities() const {
	return selectedEntities;
}

bool Editor::isActive() const {
	return !isControllingInViewPort;
}

bool Editor::isEntityValid(entt::entity entity) const {
	return engine.ecs.registry.all_of<EntityData, Transform>(entity);
}

void Editor::deleteEntity(entt::entity entity) {
	if (!engine.ecs.registry.valid(entity)) {
		return;
	}

	ImGuizmo::Enable(false); 
	ImGuizmo::Enable(true);   

	engine.ecs.registry.destroy(entity);
}

// Our main bulk of code should go here, in the main function.
void Editor::main() {
	// Verify the validity of selected and hovered entities.
	handleEntityValidity();

	ImGui::ShowDemoWindow();
	
	gameViewPort.update();
	componentInspector.update();
	assetManagerUi.update();
	hierarchyList.update();

	handleEntityHovering();
	updateMaterialMapping();
	sandboxWindow();
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

void Editor::handleEntityValidity() {
	if (!isEntityValid(hoveringEntity) && hoveringEntity != entt::null) {
		deleteEntity(hoveringEntity);
		hoveringEntity = entt::null;
	}

	bool foundInvalidSelectedEntity = false;

	for (entt::entity entity : selectedEntities) {
		if (isEntityValid(entity)) {
			continue;
		}

		// we found some invalid entity.
		if (entity != entt::null) {
			deleteEntity(entity);
			foundInvalidSelectedEntity = true;
		}
	}

	// deselect all entities.
	if (foundInvalidSelectedEntity) {
		selectEntities({}); // by selecting 0 entities haha
		//ImGuizmo::
	}
}

void Editor::handleEntityHovering() {
	if (!isActive()) {
		return;
	}

	if (ImGuizmo::IsUsing() || ImGuizmo::IsOver()) {
		return;
	}

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

		// dont render outline
		if (!isEntitySelected(hoveringEntity)) {
			meshRenderer.toRenderOutline = false;
		}
	}

	hoveringEntity = newHoveringEntity;
}

// handles object picker in game viewport
void Editor::handleEntitySelection() {	
	if (!gameViewPort.isHoveringOver) {
		return;
	}
	
	// hovered entity has already been selected.
	if (isEntitySelected(hoveringEntity)) {
		return;
	}

	// not actually trying to deselect anything, am using imguizmo controls.
	if (ImGuizmo::IsUsing() || ImGuizmo::IsOver()) {
		return;
	}

	toOutline(selectedEntities, false);
	selectedEntities.clear();

	if (hoveringEntity == entt::null) {
		return;
	}

	selectedEntities.push_back(hoveringEntity);
	toOutline(selectedEntities, true);
}

// throw all your ooga booga testing code here code quality doesnt matter
void Editor::sandboxWindow() {

	ImGui::Begin("ooga booga sandbox area");

	entt::registry& registry = engine.ecs.registry;

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

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f / 300.f, 1.f / 300.f, 1.f / 300.f}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<EntityData>(entity, EntityData{ "My 3D Model " + std::to_string(zPos) });
		zPos -= 2.f;

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
	std::string testScript{ engine.scriptingAPIManager.getAvailableScripts()[0] };
	if (ImGui::Button(testScript.c_str())) {
		entt::entity testSubject{ 0 };

		if (registry.valid(testSubject)) {
			engine.scriptingAPIManager.loadScriptIntoAPI((unsigned int)testSubject, testScript.c_str());
		}
	}

	ImGui::Text(ICON_FA_ARROW_RIGHT " icon test!!");

	ImGui::End();

	if (glfwGetKey(engine.window.getGLFWwindow(), GLFW_KEY_0)) {
		registry.get<Transform>(entt::entity{ 0 }).position.z += 0.01f;
	}

	if (glfwGetKey(engine.window.getGLFWwindow(), GLFW_KEY_1)) {
		registry.get<Transform>(entt::entity{ 1 }).localPosition.z -= 1;
	}
}

void Editor::toOutline(std::vector<entt::entity> const& entities, bool toOutline) const {
	entt::registry& registry = engine.ecs.registry;

	for (entt::entity entity : entities) {
		MeshRenderer* meshRenderer = registry.try_get<MeshRenderer>(entity);

		if (meshRenderer) {
			meshRenderer->toRenderOutline = toOutline;
		}
	}
}

Editor::~Editor() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

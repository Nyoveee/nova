#include "Engine/ScriptingAPIManager.h"
#include "Engine/window.h"
#include "Engine/engine.h"

#include <glad/glad.h>

#include "IconsFontAwesome6.h"

#include "imgui.h"
#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Audio/audioSystem.h"
#include "AssetManager/assetManager.h"
#include "ECS/ECS.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"
#include "Navigation/navMeshGeneration.h"
#include "Serialisation/serialisation.h"

#include "editor.h"
#include "themes.h"
#include "model.h"

#include "ECS/component.h"

#include <GLFW/glfw3.h>
#include <ranges>
#include <Windows.h>
#include <tracyprofiler/tracy/Tracy.hpp>

constexpr float baseFontSize = 15.0f;
constexpr const char* fontFileName = 
	"System\\Font\\"
	"NotoSans-Medium.ttf";

Editor::Editor(Window& window, Engine& engine, InputManager& inputManager, AssetManager& assetManager, ResourceManager& resourceManager) :
	window							{ window },
	engine							{ engine },
	assetManager					{ assetManager },
	resourceManager					{ resourceManager },
	inputManager					{ inputManager },
	gameViewPort					{ *this },
	componentInspector				{ *this },
	assetViewerUi					{ assetManager, resourceManager },
	assetManagerUi					{ *this, assetViewerUi },
	navMeshGenerator				{ *this },
	navigationWindow				{ *this, engine.navigationSystem, navMeshGenerator },
	navBar							{ *this },
	isControllingInViewPort			{ false },
	hoveringEntity					{ entt::null },
	inSimulationMode				{ false },
	isThereChangeInSimulationMode	{ false }
{
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

	inputManager.subscribe<CopyEntity>(
		[&](CopyEntity) {
			if (selectedEntities.size()) {
				for (entt::entity entity : selectedEntities) {
					copiedEntityVec.push_back(entity);
				}
			}
		}
	);

	inputManager.subscribe<PasteEntity>(
		[&](PasteEntity) {
			if (!copiedEntityVec.empty()) {
				engine.ecs.copyEntities<ALL_COMPONENTS>(copiedEntityVec);
				copiedEntityVec.clear();
			}
		}
	);

	if (engine.ecs.sceneManager.hasNoSceneSelected()) {
		gameViewPort.controlOverlay.setNotification("No scene selected. Select a scene from the content browser.", FOREVER);
	}
}

void Editor::update(float dt, std::function<void(bool)> changeSimulationCallback) {
	ZoneScopedC(tracy::Color::Orange);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	main(dt);
	assetManager.update();

	// inform the engine if there is a change in simulation mode.
	if (isThereChangeInSimulationMode) {
		changeSimulationCallback(inSimulationMode);
		isThereChangeInSimulationMode = false;
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (engine.toDebugRenderNavMesh) {
		ResourceID navMeshId = engine.navigationSystem.getNavMeshId();
		auto&& [navMesh, _] = resourceManager.getResource<NavMesh>(navMeshId);

		if (navMesh && navMesh->navMesh) {
			engine.renderer.renderNavMesh(*navMesh->navMesh);
		}
	}
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

	engine.ecs.deleteEntity(entity);
}

// Our main bulk of code should go here, in the main function.
void Editor::main(float dt) {
	// Verify the validity of selected and hovered entities.
	handleEntityValidity();

	ImGui::ShowDemoWindow();
	
	gameViewPort.update(dt);
	assetManagerUi.update();
	navBar.update();
	assetViewerUi.update();
	navigationWindow.update();

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
		auto [model, _] = resourceManager.getResource<Model>(modelRenderer.modelId);

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

	glm::vec2 normalisedPos = (engine.window.getClipSpacePos() + 1.f) / 2.f;

	if (	
			normalisedPos.x < 0.f
		||	normalisedPos.x >= 1.f
		||	normalisedPos.y < 0.f
		||	normalisedPos.y >= 1.f
	) {
		return;
	}

	entt::entity newHoveringEntity =
		static_cast<entt::entity>(engine.renderer.getObjectId(normalisedPos));

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

	if (ImGui::Button("SFX Audio Test"))
	{
		engine.audioSystem.playSFX( engine.audioSystem.getResourceId("SFX_AudioTest1"), 0.0f, 0.0f, 0.0f);
	}

	if (ImGui::Button("BGM Audio Test"))
	{
		engine.audioSystem.playBGM( engine.audioSystem.getResourceId("BGM_AudioTest") );
	}

	if (ImGui::Button("BGM Audio Test 2"))
	{
		engine.audioSystem.playBGM( engine.audioSystem.getResourceId("BGM_AudioTest2") );	
	}

	if (ImGui::Button("Stop Audio Test"))
	{
		// Stops all audio channels that has the same string ID as "SFX_AudioTest1"
		engine.audioSystem.StopAudio( engine.audioSystem.getResourceId("SFX_AudioTest1") );
	}

	entt::registry& registry = engine.ecs.registry;

	static bool wireFrameMode = false;
	if (ImGui::Button("Profiler")) {
		launchProfiler();
	}
	if (ImGui::Button("Wireframe mode.")) {
		wireFrameMode = !wireFrameMode;
		engine.renderer.enableWireframeMode(wireFrameMode);
	}

	if (ImGui::Button("recompile shaders")) {
		engine.renderer.recompileShaders();
	}

	if (ImGui::Button("Navigation Debug")) {
		engine.navigationSystem.NavigationDebug();
	}

	ImGui::End();
}

void Editor::launchProfiler()
{
	// Launch the profiler server connecting to the engine client
	// Todo: If the profiler is running, this should not be called
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	TCHAR path[]{ TEXT("ExternalApplication/tracy-profiler.exe") };
	CreateProcess(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
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

void Editor::startSimulation() {
	if (inSimulationMode) {
		return; // already in simulation mode.
	}

	engine.startSimulation();
	inSimulationMode = true;
	isThereChangeInSimulationMode = true;
}

void Editor::stopSimulation() {
	if (!inSimulationMode) {
		return; // already not in simulation mode.
	}

	engine.stopSimulation();
	inSimulationMode = false;
	isThereChangeInSimulationMode = true;
}

bool Editor::isInSimulationMode() const {
	return inSimulationMode;
}

Editor::~Editor() {
	
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	ResourceID id = engine.ecs.sceneManager.getCurrentScene();
	AssetFilePath const* filePath = assetManager.getFilepath(id);
	if (filePath) {
		Serialiser::serialiseScene(engine.ecs, filePath->string.c_str());
	}
}

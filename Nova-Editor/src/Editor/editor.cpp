#include "Engine/ScriptingAPIManager.h"

#include <glad/glad.h>

#include "IconsFontAwesome6.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Audio/audioSystem.h"
#include "Engine/window.h"
#include "Component/ECS.h"
#include "Engine/engine.h"
#include "InputManager/inputManager.h"
#include "ResourceManager/resourceManager.h"

#include "editor.h"
#include "themes.h"
#include "model.h"

#include "Component/component.h"

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
	assetManagerUi					{ *this },
	hierarchyList					{ *this },
	debugUi							{ *this },
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
}

void Editor::update(std::function<void(bool)> changeSimulationCallback) {
	ZoneScopedC(tracy::Color::Orange);
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	main();

	// inform the engine if there is a change in simulation mode.
	if (isThereChangeInSimulationMode) {
		changeSimulationCallback(inSimulationMode);
		isThereChangeInSimulationMode = false;
	}

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

	engine.ecs.deleteEntity(entity);
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
	debugUi.update();

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

	if (ImGui::Button("SFX Audio Test"))
	{
		engine.audioSystem.playSFX(ResourceID{ 2925900838188707808 }, 0.0f, 0.0f, 0.0f);
	}

	if (ImGui::Button("BGM Audio Test"))
	{
		engine.audioSystem.playBGM(ResourceID{ 12788208105300663898 });
	}

	if (ImGui::Button("BGM Audio Test 2"))
	{
		engine.audioSystem.playBGM(ResourceID{ 15231313946966985452 });
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

	ImGui::Text("Camera Speed: %.2f", engine.cameraSystem.getCameraSpeed());

	static float zPos = 0;

	if (ImGui::Button("(+) Add 3D model")) {
		auto entity = registry.create();

		Transform transform = {
			{0.f, 0.f, zPos},
			{1.f / 300.f, 1.f / 300.f, 1.f / 300.f}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<EntityData>(entity, EntityData{ "My 3D Model " + std::to_string(zPos) });
		registry.emplace<Rigidbody>(entity, Rigidbody{ JPH::EMotionType::Dynamic, Rigidbody::Layer::Moving });
		registry.emplace<BoxCollider>(entity, BoxCollider{ { 300.f, 200.f, 300.f } });

		zPos -= 2.f;

		std::unordered_map<MaterialName, Material> materials;

		ResourceID modelAsset{ 1857211565665677573 };

		materials["Table_frame_mtl"] = { Material::Pipeline::BlinnPhong, ResourceID{ 11363069911457047527 }, Material::Config{ 0.5f, 0.f, 0.f }, ResourceID{ 16882922978321767798 } };
		materials["Table_top_mtl"] = { Material::Pipeline::BlinnPhong, ResourceID{ 1363583670394709573 }, Material::Config{ 0.5f, 0.f, 0.f }, ResourceID{ 11291724444234068892 } };

		registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });
	}

	if (ImGui::Button("(+) Add Light")) {
		auto entity = registry.create();

		Transform transform = {
			{0.f, 2.f, 5.f},
			{0.2f, 0.2f, 0.2f}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<EntityData>(entity, EntityData{ "Light" });
		registry.emplace<Light>(entity, Light{ Color{1.f, 1.f, 1.f}, 1.f, Light::Type::PointLight });

		std::unordered_map<MaterialName, Material> materials;

		ResourceID modelAsset{ 12932563721038612588 };
		materials["Material"] = { Material::Pipeline::Color, Color{1.f, 1.f, 1.f}};

		registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });
	}

	if (ImGui::Button("(+) Add Floor")) {
		auto entity = registry.create();

		Transform transform = {
			{0.f, -2.f, 0.f},
			{10.f, 1.f, 10.f}
		};

		registry.emplace<Transform>(entity, std::move(transform));
		registry.emplace<EntityData>(entity, EntityData{ "Floor" });
		registry.emplace<Rigidbody>(entity, Rigidbody{ JPH::EMotionType::Static, Rigidbody::Layer::NonMoving });
		registry.emplace<BoxCollider>(entity, BoxCollider{});

		std::unordered_map<MaterialName, Material> materials;

		ResourceID modelAsset{ 12932563721038612588 };
		materials["Material"] = { Material::Pipeline::BlinnPhong, Color{0.1f, 0.1f, 0.1f}};

		registry.emplace<MeshRenderer>(entity, MeshRenderer{ modelAsset, materials });
	}

	if (ImGui::Button("(+) Add Skybox")) {
		auto entity = registry.create();

		registry.emplace<Transform>(entity, Transform{});
		registry.emplace<EntityData>(entity, EntityData{ "Skybox" });
		registry.emplace<SkyBox>(entity, SkyBox{ ResourceID{ 12369249828857649982 } });
	}

	if (ImGui::Button("recompile shaders")) {
		engine.renderer.recompileShaders();
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
}

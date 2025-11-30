#include "Engine/ScriptingAPIManager.h"
#include "Engine/window.h"
#include "Engine/engine.h"

#include <glad/glad.h>

#include "IconsFontAwesome6.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_node_editor.h"
#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Audio/audioSystem.h"
#include "AssetManager/assetManager.h"
#include "ECS/ECS.h"
#include "InputManager/inputManager.h"
#include "InputManager/inputEvent.h"
#include "ResourceManager/resourceManager.h"
#include "Navigation/navMeshGeneration.h"
#include "Serialisation/serialisation.h"
#include "Engine/prefabManager.h"

#include "editor.h"
#include "themes.h"
#include "model.h"

#include "component.h"

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
	editorViewPort					{ *this },
	uiViewPort						{ *this },
	componentInspector				{ *this },
	assetViewerUi					{ *this, assetManager, resourceManager },
	assetManagerUi					{ *this, assetViewerUi },
	navMeshGenerator				{ *this },
	navigationWindow				{ *this, engine.navigationSystem, navMeshGenerator },
	navBar							{ *this },
	animationTimeLine				{ *this },
	animatorController				{ *this },
	gameConfigUI					{ *this },
	editorConfigUI					{ *this },
	isControllingInViewPort			{ false },
	hoveringEntity					{ entt::null },
	inSimulationMode				{ false }
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
			if (editorViewPort.isHoveringOver) {
				toggleViewPortControl(true);
				ImGui::SetWindowFocus(nullptr);
			}
		},
		[&](ToggleEditorControl) {
			if(isControllingInViewPort) toggleViewPortControl(false);
		}
	);

	inputManager.subscribe<ToSelectGameObject>(
		[&](ToSelectGameObject) {
			handleEntitySelection();
		}
	);

	inputManager.subscribe<DeleteSelectedEntity>(
		[&](DeleteSelectedEntity) {
			if ((!editorViewPort.isHoveringOver || !editorViewPort.isActive) && !navBar.hierarchyList.isHovering) {
				return;
			}

			// @TODO: Confirmation prompt LMAOO
			for (entt::entity entity : selectedEntities) {
				deleteEntity(entity);
			}
		}
	);

	inputManager.subscribe<CopyEntity>(
		[&](CopyEntity) {
			if (selectedEntities.size() && !ImGui::IsAnyItemActive()) {
				copiedEntityVec = selectedEntities;
			}
		}
	);

	inputManager.subscribe<PasteEntity>(
		[&](PasteEntity) {
			if (!copiedEntityVec.empty() && !ImGui::IsAnyItemActive()) {
				engine.ecs.copyVectorEntities(copiedEntityVec);
			}
		}
	);

	inputManager.subscribe<FocusSelectedEntity>(
		std::bind(&Editor::handleFocusOnSelectedEntity, this, std::placeholders::_1)
	);

	inputManager.subscribe<EditorWantsToControlMouse>(
		[&](EditorWantsToControlMouse) {
			engine.editorControlMouse(true);
		}
	);

	inputManager.subscribe<ToEnableCursor>(
		[&](ToEnableCursor toEnable) {
			if (toEnable == ToEnableCursor::Enable) {
				io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			}
			else {
				io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		}
	);

	inputManager.subscribe<ScriptCompilationStatus>(
		[&](ScriptCompilationStatus status) {
			if (status == ScriptCompilationStatus::Failure) {
				editorViewPort.controlOverlay.setNotification("Script compilation has failed!", FOREVER);
			}
			else {
				editorViewPort.controlOverlay.clearNotification();
			}
		}
	);

	if (engine.ecs.sceneManager.hasNoSceneSelected()) {
		editorViewPort.controlOverlay.setNotification("No scene selected. Select a scene from the content browser.", FOREVER);
	}

#if false
	//check if there is a prefab in the scene, if there is, update the prefabManager
	// entt::registry& prefabRegistry = engine.prefabManager.getPrefabRegistry();
	std::unordered_map<ResourceID, entt::entity> prefabMap = engine.prefabManager.getPrefabMap();

	entt::registry& registry = engine.ecs.registry;
	for (entt::entity entity : registry.view<entt::entity>()) {
		EntityData* entityData = registry.try_get<EntityData>(entity);
		if (entityData->prefabID != INVALID_RESOURCE_ID) {

			if (prefabMap.find(entityData->prefabID) == prefabMap.end()) {
				engine.prefabManager.loadPrefab(entityData->prefabID);
			}
		}
	}
#endif
}

void Editor::update(float dt) {
	imguiCounter = 0;

	ZoneScopedC(tracy::Color::Orange);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	main(dt);
	assetManager.update();

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
	selectedEntities = std::move(entities);
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

	engine.ecs.deleteEntity(entity);
}

// Our main bulk of code should go here, in the main function.
void Editor::main(float dt) {
	// Verify the validity of selected and hovered entities.
	handleEntityValidity();
	
	gameViewPort.update(dt);
	editorViewPort.update(dt);
	uiViewPort.update();
	assetManagerUi.update();
	navBar.update();
	assetViewerUi.update();
	//navigationWindow.update();
	animationTimeLine.update(dt);
	//animatorController.update();
	//gameConfigUI.update();
	editorConfigUI.update();

	handleEntityHovering();

	engine.renderer.submitSelectedObjects(selectedEntities);
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

void Editor::handleEntityValidity() {
	if (!isEntityValid(hoveringEntity) && hoveringEntity != entt::null) {
		if (engine.ecs.registry.valid(hoveringEntity)) {
			deleteEntity(hoveringEntity);
		}
		hoveringEntity = entt::null;
	}

	bool foundInvalidSelectedEntity = false;

	for (entt::entity entity : selectedEntities) {
		if (isEntityValid(entity)) {
			continue;
		}

		// we found some invalid entity.
		if (entity != entt::null) {
			if (engine.ecs.registry.valid(entity)) {
				deleteEntity(entity);
			}
			foundInvalidSelectedEntity = true;
		}
	}

	// deselect all entities.
	if (foundInvalidSelectedEntity) {
		selectEntities({}); // by selecting 0 entities haha
	}
}

void Editor::handleEntityHovering() {
	if (!isActive() || !editorViewPort.isActive) {
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
	hoveringEntity = newHoveringEntity;
}

// handles object picker in game viewport
void Editor::handleEntitySelection() {	
	handleUIEntitySelection();

	if (!editorViewPort.isHoveringOver) {
		return;
	}
	
	// hovered entity has already been selected.
	if (isEntitySelected(hoveringEntity)) {
		return;
	}

	// not actually trying to deselect anything, am using imguizmo controls.
	if (selectedEntities.size() && (ImGuizmo::IsUsing() || ImGuizmo::IsOver())) {
		return;
	}

	selectedEntities.clear();

	if (hoveringEntity == entt::null) {
		return;
	}

	selectedEntities.push_back(hoveringEntity);
	toOutline(selectedEntities, true);
}

void Editor::handleFocusOnSelectedEntity(FocusSelectedEntity) {
	// Only focus if we have a selected entity
	if (selectedEntities.empty()) {
		return;
	}

	if (ImGui::IsAnyItemActive()) {
		return;
	}

	// Get the first selected entity's transform
	entt::entity entity = selectedEntities[0];
	entt::registry& registry = engine.ecs.registry;

	Transform* transform = registry.try_get<Transform>(entity);
	if (!transform) {
		return;
	}

	// Focus the camera on the entity's position
	engine.cameraSystem.focusOnPosition(transform->position);
}

void Editor::handleUIEntitySelection() {
	if (!uiViewPort.isHoveringOver) {
		return;
	}

	ImVec2 mouseRelativeToViewPort = ImGui::GetMousePos();

	// Calculate the mouse position relative to the game's viewport.
	mouseRelativeToViewPort -= uiViewPort.windowTopLeft;
	mouseRelativeToViewPort /= ImVec2{ uiViewPort.windowDimension.x, uiViewPort.windowDimension.y };

	// Flip y..
	mouseRelativeToViewPort.y = 1 - mouseRelativeToViewPort.y;

	if (
			mouseRelativeToViewPort.x < 0.f
		||	mouseRelativeToViewPort.x >= 1.f
		||	mouseRelativeToViewPort.y < 0.f
		||	mouseRelativeToViewPort.y >= 1.f
	) {
		return;
	}

	entt::entity selectedUIEntity = static_cast<entt::entity>(engine.renderer.getObjectUiId(glm::vec2{ mouseRelativeToViewPort.x, mouseRelativeToViewPort.y }));

	// not actually trying to deselect anything, am using imguizmo controls.
	if (selectedEntities.size() && (ImGuizmo::IsUsing() || ImGuizmo::IsOver())) {
		return;
	}

	selectedEntities.clear();

	if (selectedUIEntity == entt::null) {
		return;
	}

	selectedEntities.push_back(selectedUIEntity);
}

void Editor::displayEntityScriptDropDownList(ResourceID id, const char* labelName, entt::entity entity, std::function<void(ResourceID)> const& onClickCallback) {
	char const* selectedAssetName = "";

	ImGui::PushID(++imguiCounter);

	auto namePtr = assetManager.getName(id);
	selectedAssetName = namePtr ? namePtr->c_str() : "No resource selected.";

	// Uppercase search query..
	// Case insensitive searchQuery..
	uppercaseSearchQuery.clear();
	std::transform(assetSearchQuery.begin(), assetSearchQuery.end(), std::back_inserter(uppercaseSearchQuery), [](char c) { return static_cast<char>(std::toupper(c)); });

	// get all scripts of the entity..
	Scripts* scripts = engine.ecs.registry.try_get<Scripts>(entity);

	if (!scripts) {
		ImGui::BeginDisabled();
		if (ImGui::BeginCombo(labelName, "No script component.")) { ImGui::EndCombo(); }
		ImGui::EndDisabled();

		ImGui::PopID();
		return;
	}

	if (ImGui::BeginCombo(labelName, selectedAssetName)) {
		ImGui::InputText("Search", &assetSearchQuery);

		for (auto&& scriptData : scripts->scriptDatas) {
			std::string const* assetName = assetManager.getName(scriptData.scriptId);

			if (!assetName) {
				continue;
			}

			// Let's upper case our component name..
			uppercaseAssetName.clear();
			std::transform(assetName->begin(), assetName->end(), std::back_inserter(uppercaseAssetName), [](char c) { return static_cast<char>(std::toupper(c)); });

			// attempt to find asset..
			if (uppercaseAssetName.find(uppercaseSearchQuery) == std::string::npos) {
				continue;
			}

			ImGui::PushID(static_cast<int>(static_cast<std::size_t>(scriptData.scriptId)));

			if (ImGui::Selectable(assetName->empty() ? "<no name>" : assetName->c_str(), id == scriptData.scriptId)) {
				onClickCallback(scriptData.scriptId);
			}

			ImGui::PopID();
		}

		ImGui::EndCombo();
	}

#if false
	// handle drag and drop..
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM")) {
			auto&& [draggedId, name] = *((std::pair<std::size_t, const char*>*)payload->Data);

			if (resourceManager.isResource<T>(draggedId)) {
				onClickCallback(draggedId);
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_ANCHOR_CIRCLE_XMARK)) {
		assetViewerUi.selectNewResourceId(id.value());
		assetManagerUi.displayAssetFolder(id.value());
		ImGui::SetWindowFocus(ICON_FA_AUDIO_DESCRIPTION " Asset Viewer");
	}
#endif

	ImGui::PopID();
}

void Editor::displayAllEntitiesDropDownList(const char* labelName, entt::entity selectedEntity, std::function<void(entt::entity)> const& onClickCallback) {
	entt::registry& registry = [&]() -> entt::registry& {
		if (displayingPrefabScripts) {
			return engine.prefabManager.getPrefabRegistry();
		}
		else {
			return engine.ecs.registry;
		}
	}();

	ImGui::PushID(++imguiCounter);

	EntityData* selectedEntityData = registry.try_get<EntityData>(selectedEntity);
	const char* selectedEntityName = selectedEntityData ? selectedEntityData->name.c_str() : "<None>";

	// Uppercase search query..
	// Case insensitive searchQuery..
	uppercaseEntitySearchQuery.clear();
	std::transform(entitySearchQuery.begin(), entitySearchQuery.end(), std::back_inserter(uppercaseEntitySearchQuery), [](char c) { return static_cast<char>(std::toupper(c)); });

	if (ImGui::BeginCombo(labelName, selectedEntityName)) {
		ImGui::InputText("Search", &entitySearchQuery);

		ImGui::PushID(-1);

		if (ImGui::Selectable("<None>", false)) {
			onClickCallback(entt::null);
		}

		ImGui::PopID();

		for (auto&& [entityId, entityData] : registry.view<EntityData>().each()) {
			// Let's upper case our entity name..
			uppercaseEntityName.clear();
			std::transform(entityData.name.begin(), entityData.name.end(), std::back_inserter(uppercaseEntityName), [](char c) { return static_cast<char>(std::toupper(c)); });

			// attempt to find entity..
			if (uppercaseEntityName.find(uppercaseEntitySearchQuery) == std::string::npos) {
				continue;
			}

			ImGui::PushID(static_cast<int>(entityId));

			if (ImGui::Selectable(entityData.name.empty() ? "<no name>" : entityData.name.c_str(), selectedEntity == entityId)) {
				onClickCallback(entityId);
			}

			ImGui::PopID();
		}

		ImGui::EndCombo();
	}

	if (ImGui::BeginDragDropTarget()) {

		if (displayingPrefabScripts) {
			if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("PREFAB_HIERARCHY_ITEM")) {
				onClickCallback(*((entt::entity*)payload->Data));
			}
		}
		else {
			if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ITEM")) {
				onClickCallback(*((entt::entity*)payload->Data));
			}
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();
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

void Editor::toOutline(std::vector<entt::entity> const&, bool) const {
#if false
	entt::registry& registry = engine.ecs.registry;

	for (entt::entity entity : entities) {
		MeshRenderer* meshRenderer = registry.try_get<MeshRenderer>(entity);

		if (meshRenderer) {
			meshRenderer->toRenderOutline = toOutline;
		}
	}
#endif
}

void Editor::toControlMouse(bool toControl) {
	// legacy reasons..
	engine.editorControlMouse(toControl);
}

void Editor::startSimulation() {
	if (inSimulationMode) {
		return; // already in simulation mode.
	}

	// editor doesn't need to control the mouse anymore..
	toControlMouse(false);

	engine.startSimulation();

	// We serialise everything, resources to current scene when starting a simulation..
	ResourceID id = engine.ecs.sceneManager.getCurrentScene();
	AssetFilePath const* filePath = assetManager.getFilepath(id);
	Serialiser::serialiseScene(engine.ecs.registry, engine.ecs.sceneManager.layers, filePath->string.c_str());

#if 0
	assetManager.serialiseResources();
#endif

	inSimulationMode = true;
}

void Editor::stopSimulation() {
	if (!inSimulationMode) {
		return; // already not in simulation mode.
	}

	toControlMouse(true);
	engine.stopSimulation();
	inSimulationMode = false;
}

bool Editor::isInSimulationMode() const {
	return inSimulationMode;
}

void Editor::displayEntityHierarchy(
	entt::registry& registry,
	entt::entity entity,
	bool toRecurse,
	const char* dragAndDropIdentifier,
	std::function<void(std::vector<entt::entity>)> const& onClickFunction,
	std::function<bool(entt::entity)> const& selectedPredicate
) {
	constexpr ImVec4 prefabColor = ImVec4(0.5f, 1.f, 1.f, 1.f);
	constexpr ImVec4 grayColor = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
	constexpr ImVec4 whiteColor = ImVec4(1.f, 1.f, 1.f, 1.f);

	if (!registry.valid(entity)) {
		return;
	}

	EntityData const& entityData = registry.get<EntityData>(entity);

	bool toDisplayTreeNode = false;

	ImGui::PushID(static_cast<unsigned>(entity));

	// We use IILE to conditionally initialise a variable :)
	ImVec4 color = [&]() {
		// If the current entity is disabled, render gray..
		if (!entityData.isActive) {
			return grayColor;
		}

		// Else, render green or white 
		return [&]() {
			EntityData const* root = &entityData;

			while (root->parent != entt::null) {
				if (root->prefabID != INVALID_RESOURCE_ID)
					return prefabColor;

				root = registry.try_get<EntityData>(root->parent);
			}

			if (root->prefabID != INVALID_RESOURCE_ID) {
				return prefabColor;
			}
			else {
				return whiteColor;
			}
		}();
	}();

	ImGui::PushStyleColor(ImGuiCol_Text, color);

	if (!toRecurse || entityData.children.empty()) {
		ImGui::Indent(27.5f);
		if (ImGui::Selectable((
			(entityData.prefabID == INVALID_RESOURCE_ID ? ICON_FA_CUBE : ICON_FA_CUBE)
			+ std::string{ " " } + entityData.name).c_str(), selectedPredicate(entity))) {
			onClickFunction({ entity });
		}

		// Check for double-click to focus on entity
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			Transform* transform = registry.try_get<Transform>(entity);
			if (transform) {
				engine.cameraSystem.focusOnPosition(transform->position);
			}
		}
		ImGui::Unindent(27.5f);
	}
	else {
		// Display children recursively..
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

		if (selectedPredicate(entity)) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		toDisplayTreeNode = ImGui::TreeNodeEx((
			(entityData.prefabID == INVALID_RESOURCE_ID ? ICON_FA_CUBE : ICON_FA_CUBE)
			+ std::string{ " " } + entityData.name).c_str(), flags
		);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			onClickFunction({ entity });
		}

		// Check for double-click to focus on entity
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
			Transform* transform = registry.try_get<Transform>(entity);
			if (transform) {
				engine.cameraSystem.focusOnPosition(transform->position);
			}
		}
	}

	ImGui::PopStyleColor();

	// I want my widgets to be draggable, providing the entity id as the payload.
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload(dragAndDropIdentifier, &entity, sizeof(entt::entity*));

		// Draw tooltip-style preview while dragging
		ImGui::Text(entityData.name.c_str());

		ImGui::EndDragDropSource();
	}

	// I want all my widgets to be a valid drop target.
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload(dragAndDropIdentifier)) {
			entt::entity childEntity = *((entt::entity*)payload->Data);
			engine.ecs.setEntityParent(childEntity, entity, false, registry);
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();

	// recursively displays tree hierarchy..
	if (toDisplayTreeNode) {
		for (entt::entity child : entityData.children) {
			displayEntityHierarchy(registry, child, toRecurse, dragAndDropIdentifier, onClickFunction, selectedPredicate);
		}

		ImGui::TreePop();
	}
}

void Editor::loadScene(ResourceID sceneId) {
	if (engine.isInSimulationMode()) {
		return;
	}

	AssetFilePath const* filePath = assetManager.getFilepath(engine.ecs.sceneManager.getCurrentScene());

	if (filePath) {
		Serialiser::serialiseScene(engine.ecs.registry, engine.ecs.sceneManager.layers, filePath->string.c_str());
	}

	engine.ecs.sceneManager.loadScene(sceneId);
	editorViewPort.controlOverlay.clearNotification();

	// engine.prefabManager.prefabBroadcast();

	// deselect entity.
	selectEntities({});
}

void Editor::unpackPrefab(EntityData& entityData) {
	entityData.prefabID = TypedResourceID<Prefab>{ INVALID_RESOURCE_ID };

	for (entt::entity child : entityData.children) {
		EntityData& childEntityData = engine.ecs.registry.get<EntityData>(child);
		unpackPrefab(childEntityData);
	}
}

Editor::~Editor() {
	
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	ResourceID id = engine.ecs.sceneManager.getCurrentScene();
	AssetFilePath const* filePath = assetManager.getFilepath(id);

	if (filePath && !isInSimulationMode()) {
		Serialiser::serialiseScene(engine.ecs.registry, engine.ecs.sceneManager.layers, filePath->string.c_str());
	}
	if (!isInSimulationMode()) {
		entt::registry& prefabRegistry = engine.prefabManager.getPrefabRegistry();
		std::unordered_map<ResourceID, entt::entity> prefabMap = engine.prefabManager.getPrefabMap();

		for (auto& pair : prefabMap) {
			auto descriptor = assetManager.getDescriptor(pair.first);
			if (descriptor != nullptr) {
				std::ofstream assetFile{ descriptor->filepath.string};
				Serialiser::serialisePrefab(prefabRegistry, pair.second, assetFile);
			}
		}
	}

}

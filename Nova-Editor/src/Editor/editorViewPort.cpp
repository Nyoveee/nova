#include "Engine/engine.h"
#include "Engine/window.h"
#include "Engine/prefabManager.h"
#include "component.h"

#include "editor.h"
#include "editorViewPort.h"
#include "Serialisation/serialisation.h"
#include "IconsFontAwesome6.h"
#include "AssetManager/assetManager.h"
#include "video.h"

EditorViewPort::EditorViewPort(Editor& editor) :
	editor					{ editor },
	engine					{ editor.engine },
	gizmo					{ editor, engine.ecs },
	controlOverlay			{ editor, gizmo },
	isHoveringOver			{ false }
{}

void EditorViewPort::update(float dt) {
	engine.renderer.isEditorScreenShown = ImGui::Begin(ICON_FA_GAMEPAD " Editor");
	isHoveringOver = ImGui::IsWindowHovered();
	isActive = ImGui::IsWindowFocused();
	
	// Get ImGui window's top left and bottom right.
	ImVec2 gameWindowTopLeft = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos();
	ImVec2 gameWindowBottomRight = ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos();

	// Dimension of the actual window.
	float windowWidth = gameWindowBottomRight.x - gameWindowTopLeft.x;
	float windowHeight = gameWindowBottomRight.y - gameWindowTopLeft.y;

	// Specified dimension for our game.
	float gameWidth = static_cast<float>(engine.getGameWidth());
	float gameHeight = static_cast<float>(engine.getGameHeight());

	float ratio = 1.f;
	float viewportWidth;
	float viewportHeight;

	// We need to shrink this specified dimension for our game to fit within the actual window.
	if (gameWidth > windowWidth) {
		ratio = gameWidth / windowWidth;
		viewportWidth = windowWidth;
		viewportHeight = gameHeight / ratio;
	}
	else {
		viewportWidth = gameWidth;
		viewportHeight = gameHeight;
	}

	if (viewportHeight > windowHeight) {
		ratio = viewportHeight / windowHeight;
		viewportWidth /= ratio;
		viewportHeight = windowHeight;
	}

	// We adjust our game window's top left and bottom right position to centralise our game view port.
	float centerXOffset = (windowWidth - viewportWidth) / 2.f;
	float centerYOffset = (windowHeight - viewportHeight) / 2.f;

	gameWindowTopLeft.x += centerXOffset;
	gameWindowTopLeft.y += centerYOffset;
	gameWindowBottomRight = { gameWindowTopLeft.x + viewportWidth, gameWindowTopLeft.y + viewportHeight };

	// Retrieve main texture from main frame buffer in renderer and put it in imgui draw list.
	ImTextureID textureId = engine.renderer.getEditorFrameBufferTexture();
	ImGui::GetWindowDrawList()->AddImage(textureId, gameWindowTopLeft, gameWindowBottomRight, { 0, 1 }, { 1, 0 });
	
	engine.window.setGameViewPort({ 
		static_cast<int>(gameWindowTopLeft.x), 
		static_cast<int>(gameWindowTopLeft.y),
		static_cast<int>(viewportWidth), 
		static_cast<int>(viewportHeight) 
	});

	gizmo.update(gameWindowTopLeft.x, gameWindowTopLeft.y, viewportWidth, viewportHeight);
	controlOverlay.update(dt, gameWindowTopLeft.x, gameWindowTopLeft.y, viewportWidth, viewportHeight);

	ImGui::Dummy(ImGui::GetContentRegionAvail());

	// Accept scene item payload..
	if (ImGui::BeginDragDropTarget()) {
		if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("DRAGGING_ASSET_ITEM")) {
			auto&& [id, name] = *((std::pair<std::size_t, const char*>*)payload->Data);

			// handling scene drop request..
			if (editor.resourceManager.isResource<Scene>(id)) {
				editor.loadScene(id);
			}
			else if (editor.resourceManager.isResource<Prefab>(id)) {
				engine.prefabManager.instantiatePrefab(id);
				editor.selectEntities({});
			}
			else if (editor.resourceManager.isResource<Video>(id)) {
				// Create entity with VideoPlayer component
				auto entity = engine.ecs.registry.create();

				// Add EntityData
				auto& entityData = engine.ecs.registry.emplace<EntityData>(entity);
				entityData.name = name;

				// Add Transform
				engine.ecs.registry.emplace<Transform>(entity);

				// Add VideoPlayer
				auto& videoPlayer = engine.ecs.registry.emplace<VideoPlayer>(entity);
				videoPlayer.videoId = TypedResourceID<Video>{ id };

				// Load the video resource
				auto [video, refCount] = editor.resourceManager.getResource<Video>(id);
				if (video) {
					video->load();
				}

				// Select the new entity
				editor.selectEntities({ entity });
			}
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::End();
	
}

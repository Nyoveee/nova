#include "Engine/engine.h"
#include "Engine/window.h"

#include "editor.h"
#include "UIViewPort.h"
#include "Serialisation/serialisation.h"
#include "IconsFontAwesome6.h"
#include "AssetManager/assetManager.h"

UIViewPort::UIViewPort(Editor& editor) :
	editor					{ editor },
	engine					{ editor.engine },
	gizmo					{ editor, engine.ecs },
	controlOverlay			{ editor },
	isHoveringOver			{ false }
{}

void UIViewPort::update() {
	ImGui::Begin(ICON_FA_GAMEPAD " UIEditor");
	isHoveringOver = ImGui::IsWindowHovered();
	isActive = ImGui::IsWindowFocused();

	// Get ImGui window's top left and bottom right.
	ImVec2 uiWindowTopLeft = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos();
	ImVec2 uiWindowBottomRight = ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos();

	// Dimension of the actual window.
	float windowWidth = uiWindowBottomRight.x - uiWindowTopLeft.x;
	float windowHeight = uiWindowBottomRight.y - uiWindowTopLeft.y;

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

	uiWindowTopLeft.x += centerXOffset;
	uiWindowTopLeft.y += centerYOffset;
	uiWindowBottomRight = { uiWindowTopLeft.x + viewportWidth, uiWindowTopLeft.y + viewportHeight };

	// Retrieve main texture from main frame buffer in renderer and put it in imgui draw list.
	ImTextureID textureId = engine.renderer.getUIFrameBufferTexture();
	ImGui::GetWindowDrawList()->AddImage(textureId, uiWindowTopLeft, uiWindowBottomRight, { 0, 1 }, { 1, 0 });

	windowTopLeft = uiWindowTopLeft;
	windowDimension = { viewportWidth, viewportHeight };

	gizmo.update(uiWindowTopLeft.x, uiWindowTopLeft.y, viewportWidth, viewportHeight, true);
	
	ImGui::End();
	
}

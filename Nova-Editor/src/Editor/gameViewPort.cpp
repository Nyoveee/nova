#include "editor.h"
#include "engine.h"
#include "gameViewPort.h"

#include "IconsFontAwesome6.h"

GameViewPort::GameViewPort(Editor& editor) :
	engine					{ editor.engine },
	gizmo					{ editor, engine.ecs },
	isHoveringOver			{ false },
	mouseRelativeToViewPort {}
{}

void GameViewPort::update() {
	ImGui::Begin(ICON_FA_GAMEPAD " Game");
	isHoveringOver = ImGui::IsWindowHovered();

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
	ImTextureID textureId = engine.renderer.getMainFrameBufferTextures()[0];
	ImGui::GetWindowDrawList()->AddImage(textureId, gameWindowTopLeft, gameWindowBottomRight, { 0, 1 }, { 1, 0 });

	// Calculate the mouse position relative to the game's viewport.
	mouseRelativeToViewPort = ImGui::GetMousePos();
	mouseRelativeToViewPort -= gameWindowTopLeft;
	mouseRelativeToViewPort /= ImVec2{ viewportWidth, viewportHeight };

	// Flip y..
	mouseRelativeToViewPort.y = 1 - mouseRelativeToViewPort.y;

	gizmo.update(gameWindowTopLeft.x, gameWindowTopLeft.y, viewportWidth, viewportHeight);

	ImGui::End();
}

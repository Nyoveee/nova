#include "engine.h"
#include "gameViewPort.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

GameViewPort::GameViewPort(Engine& engine) :
	engine			{ engine },
	isHoveringOver	{ false }
{}

void GameViewPort::update() {
	ImGui::Begin("Game");
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
	float imageWidth;
	float imageHeight;

	// We need to shrink this specified dimension for our game to fit within the actual window.
	if (gameWidth > windowWidth) {
		ratio = gameWidth / windowWidth;
		imageWidth = windowWidth;
		imageHeight = gameHeight / ratio;
	}
	else {
		imageWidth = gameWidth;
		imageHeight = gameHeight;
	}

	if (imageHeight > windowHeight) {
		ratio = imageHeight / windowHeight;
		imageWidth /= ratio;
		imageHeight = windowHeight;
	}

	// We adjust our game window's top left and bottom right position to centralise our game view port.
	float centerXOffset = (windowWidth - imageWidth) / 2.f;
	float centerYOffset = (windowHeight - imageHeight) / 2.f;

	gameWindowTopLeft.x += centerXOffset;
	gameWindowTopLeft.y += centerYOffset;
	gameWindowBottomRight = { gameWindowTopLeft.x + imageWidth, gameWindowTopLeft.y + imageHeight };

	// Retrieve texture from main frame buffer in renderer and put it in imgui draw list.
	ImTextureID textureId = engine.renderer.getMainFrameBufferTexture();
	ImGui::GetWindowDrawList()->AddImage(textureId, gameWindowTopLeft, gameWindowBottomRight, { 0, 1 }, { 1, 0 });

	ImGui::End();
}

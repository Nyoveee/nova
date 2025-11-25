#include "controlOverlay.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "editor.h"
#include "Engine/engine.h"

namespace {
	void centerTextInWindow(const char* text) {
		float window_width = ImGui::GetWindowContentRegionMax().x; 
		float text_width = ImGui::CalcTextSize(text).x;

		// Calculate the X position to center the text
		float text_indentation = (window_width - text_width) * 0.5f;

		// Set the cursor position before drawing the text
		ImGui::SetCursorPosX(text_indentation);
		ImGui::Text(text);
	}
}

ControlOverlay::ControlOverlay(Editor& editor, Gizmo& gizmo) :
	editor	{ editor },
	gizmo	{ gizmo }
{}

void ControlOverlay::update(float dt, float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight) {
	displayTopControlBar(viewportPosX, viewportPosY, viewportWidth, viewportHeight);

	if (notificationText.size()) {
		displayNotification(viewportPosX, viewportPosY, viewportWidth, viewportHeight);

		timeElapsed += dt;

		if (timeElapsed > notificationDuration) {
			clearNotification();
		}
	}
}

void ControlOverlay::setNotification(std::string text, float duration) {
	notificationText = std::move(text);
	timeElapsed = 0.f;
	notificationDuration = duration;
}

void ControlOverlay::clearNotification() {
	notificationText.clear();
	timeElapsed = 0.f;
}

void ControlOverlay::displayTopControlBar(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight) {
	constexpr float overlayWidth	= 120.f;
	constexpr float overlayHeight	= 25.f;
	constexpr float topPadding		= 10.f;
	constexpr float buttonSize		= 25.f;

	if (viewportHeight < overlayHeight || viewportWidth < overlayWidth) {
		return;
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowSize({ overlayWidth, overlayHeight });
	ImVec2 topLeftPos = { viewportWidth / 2.f + viewportPosX - overlayWidth / 2.f, viewportPosY + topPadding };
	ImGui::SetNextWindowPos(topLeftPos);

	// lmao i dont know which padding affects what so i just disable all
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });

	ImGui::Begin("Overlay", nullptr, window_flags);

	if (editor.engine.ecs.sceneManager.hasNoSceneSelected()) {
		ImGui::BeginDisabled();
	}

	if (ImGui::BeginTable("##buttons", 3, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		// aint no way imgui has no way to center buttons
		float columnWidth = ImGui::GetColumnWidth();

		// Compute horizontal offset to center the button
		float offset = (columnWidth - buttonSize) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		if (ImGui::Button(editor.isInSimulationMode() ? ICON_FA_STOP : ICON_FA_PLAY, ImVec2{ buttonSize, buttonSize })) {
			editor.isInSimulationMode() ? editor.stopSimulation() : editor.startSimulation();
		}

		ImGui::TableNextColumn();

		// Compute horizontal offset to center the button
		offset = (columnWidth - buttonSize) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		if (!editor.isInSimulationMode()) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button(ICON_FA_PAUSE, ImVec2{ buttonSize, buttonSize })) {
			// not implemented. @TODO: implement pause.
		}

		if (!editor.isInSimulationMode()) {
			ImGui::EndDisabled();
		}

		ImGui::TableNextColumn();

		// Compute horizontal offset to center the button
		offset = (columnWidth - buttonSize) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		if (ImGui::Button(gizmo.mode == ImGuizmo::MODE::WORLD ? ICON_FA_GLOBE : ICON_FA_LOCATION_DOT, ImVec2{ buttonSize, buttonSize })) {
			if (gizmo.mode == ImGuizmo::MODE::WORLD) {
				gizmo.mode = ImGuizmo::MODE::LOCAL;
			}
			else {
				gizmo.mode = ImGuizmo::MODE::WORLD;
			}
		}

		ImGui::EndTable();
	}

	if (editor.engine.ecs.sceneManager.hasNoSceneSelected()) {
		ImGui::EndDisabled();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void ControlOverlay::displayNotification(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight) {
	constexpr float overlayWidth = 500.f;
	constexpr float overlayHeight = 42.f;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowBgAlpha(0.7f);
	ImGui::SetNextWindowSize({ overlayWidth, overlayHeight });
	ImVec2 topLeftPos = { viewportWidth / 2.f + viewportPosX - overlayWidth / 2.f, viewportHeight / 2.f + viewportPosY - overlayHeight / 2.f };
	ImGui::SetNextWindowPos(topLeftPos);

	ImGui::Begin("Notification Overlay", nullptr, window_flags);
	centerTextInWindow(notificationText.c_str());
	ImGui::End();
}

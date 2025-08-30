#include "controlOverlay.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "editor.h"

constexpr float overlayWidth = 70.f;
constexpr float overlayHeight = 25.f;
constexpr float topPadding = 10.f;
constexpr float buttonSize = 25.f;

ControlOverlay::ControlOverlay(Editor& editor) :
	editor { editor }
{}

void ControlOverlay::update(float viewportPosX, float viewportPosY, float viewportWidth, float viewportHeight) {
	if (viewportHeight < overlayHeight || viewportWidth < overlayWidth) {
		return;
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowSize({ overlayWidth, overlayHeight });
	ImVec2 topRightPos = { viewportWidth / 2.f + viewportPosX - overlayWidth / 2.f, viewportPosY + topPadding };
	ImGui::SetNextWindowPos(topRightPos);

	// lmao i dont know which padding affects what so i just disable all
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });

	ImGui::Begin("Overlay", nullptr, window_flags);

	if (ImGui::BeginTable("##buttons", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		// aint no way imgui has no way to center buttons
		float columnWidth = ImGui::GetColumnWidth();

		// Compute horizontal offset to center the button
		float offset = (columnWidth - buttonSize) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		if (ImGui::Button(editor.isInSimulationMode() ? ICON_FA_STOP : ICON_FA_PLAY, ImVec2{buttonSize, buttonSize})) {
			editor.isInSimulationMode() ? editor.stopSimulation() : editor.startSimulation();
		}

		ImGui::TableNextColumn();

		// aint no way imgui has no way to center buttons
		columnWidth = ImGui::GetColumnWidth();

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

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

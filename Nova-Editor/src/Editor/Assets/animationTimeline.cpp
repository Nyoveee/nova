#include "animationTimeline.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

#include "IconsFontAwesome6.h"
#include "imgui_neo_sequencer.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

constexpr int FPS = 60;
constexpr int ONE_SECOND = 1;
constexpr int HUNDRED_SECONDS = 100;

AnimationTimeLine::AnimationTimeLine(Editor& editor) :
	editor					{ editor },
	resourceManager			{ editor.resourceManager },
	viewDuration			{ ONE_SECOND },
	registry				{ editor.engine.ecs.registry },
	currentFrame			{ },
	selectedEntity			{ entt::null }
{}

void AnimationTimeLine::update() {
	ImGui::Begin(ICON_FA_TIMELINE " Sequencer");

	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	selectedEntity = editor.getSelectedEntities()[0];
	
	Sequence* sequence = registry.try_get<Sequence>(selectedEntity);

	if (!sequence) {
		ImGui::Text("Selected entity has no sequence component.");
		ImGui::End();
		return;
	}

	auto&& [sequencer, _] = resourceManager.getResource<Sequencer>(sequence->sequencerId);

	if (!sequencer) {
		ImGui::Text("Selected entity's sequence component is pointing to an invalid resource.");
		ImGui::End();
		return;
	}

	currentFrame = static_cast<int>(sequence->timeElapsed * FPS);

	ImGui::SameLine();
	displayMainPanel(*sequence, *sequencer);

	// update time elapsed :)
	if (!editor.engine.isInSimulationMode()) {
		sequence->currentFrame = currentFrame;
		sequence->timeElapsed = (float)currentFrame / FPS;
	}

	ImGui::End();
}

void AnimationTimeLine::displayMainPanel(Sequence& sequence, Sequencer& sequencer) {
	ImGui::BeginChild("Sequencer Timeline");

	displayTopPanel(sequencer);

	float lastFrameInSeconds = (float)(sequencer.data.lastFrame) / (float)FPS;

	if (viewDuration < lastFrameInSeconds) {
		viewDuration = lastFrameInSeconds;
	}

	ImGui::DragFloat("View (in seconds)", &viewDuration, 1.f, lastFrameInSeconds, HUNDRED_SECONDS);
	
	int startFrame = 0;
	int endFrame = static_cast<int>(viewDuration * FPS);

	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, {}, ImGuiNeoSequencerFlags_Selection_DisplayTimeInsteadOfFrames)) {
		if (ImGui::BeginNeoTimelineEx("Event", nullptr, ImGuiNeoTimelineFlags_Group)) {
			ImGui::EndNeoTimeLine();
		}
	
		if (ImGui::BeginNeoTimelineEx("Transform", nullptr)) {
			for (auto&& keyframes : sequencer.data.keyframes) {
				int frame = keyframes.frame;

				ImGui::PushID(frame);
				ImGui::NeoKeyframe(&frame);
				ImGui::PopID();
			}

			ImGui::EndNeoTimeLine();
		}

		ImGui::EndNeoSequencer();
	}

	displayKeyframes(sequence, sequencer);

	ImGui::EndChild();
}

void AnimationTimeLine::displayTopPanel(Sequencer& sequencer) {
	EntityData const& entityData = registry.get<EntityData>(selectedEntity);
	Transform const& transform = registry.get<Transform>(selectedEntity);

	ImGui::SeparatorText(entityData.name.c_str());

	if (editor.engine.isInSimulationMode()) {
		ImGui::BeginDisabled();
	}

	displayTopToolbar(sequencer);

	if (ImGui::Button("[+] Keyframe")) {
		sequencer.recordKeyframe(currentFrame, transform);
	}

	ImGui::SameLine();

	if (ImGui::Button("[+] Event")) {

	}

	if (editor.engine.isInSimulationMode()) {
		ImGui::EndDisabled();
	}
}

void AnimationTimeLine::displayTopToolbar([[maybe_unused]] Sequencer& sequencer) {
	constexpr float toolbarButtonSize = 30.f;
	constexpr float toolbarInputSize = 45.f;


	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	// Display the whole controls button overlay..
	if (ImGui::BeginTable("Control", 6)) {
		// Set up frame columns..
		ImGui::TableSetupColumn("Start Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Prev Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Play", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Next Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("End Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Current Frame");

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_BACKWARD_FAST)) {

		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_BACKWARD_STEP)) {

		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_PLAY)) {

		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_FORWARD_STEP)) {

		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_FORWARD_FAST)) {

		}

		ImGui::TableNextColumn();

		// stupid imgui requres the manual calculation of cursor position
		// cursor position is the main way to control drawing..
		// 1. Calculate the position for right alignment
		float column_width = ImGui::GetColumnWidth();
		float cursor_x = ImGui::GetCursorPosX();
		float item_spacing_x = ImGui::GetStyle().ItemSpacing.x;

		// Calculate the new X position so the widget sits at the right edge, accounting for spacing
		float new_cursor_x = cursor_x + (column_width - toolbarInputSize - item_spacing_x);

		// Ensure the cursor position doesn't overlap with items to the left
		if (new_cursor_x > ImGui::GetCursorPosX()) {
			ImGui::SetCursorPosX(new_cursor_x);
		}
		else {
			// If overlap would occur (e.g. column is very narrow), just align left or handle as appropriate
			ImGui::SetCursorPosX(cursor_x);
		}

		ImGui::SetNextItemWidth(toolbarInputSize);
		ImGui::InputInt("##", &currentFrame, 0, 0);

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();
}

void AnimationTimeLine::displayKeyframes(Sequence& sequence, Sequencer& sequencer) {
	if (ImGui::BeginTable("Sequencer Keyframes Table", 4)) {
		ImGui::TableSetupColumn("Keyframe", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Lerp Type", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Power", ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, 40.0f);

		// Populate table rows
		int imguiCounter = -1;

		for (auto it = sequencer.data.keyframes.begin(); it != sequencer.data.keyframes.end();) {
			auto&& keyframe = *it;

			ImGui::PushID(imguiCounter++);

			ImGui::TableNextRow();		// Start a new row

			ImGui::TableNextColumn();

			if (keyframe.copyFrame == -1) {
				keyframe.copyFrame = keyframe.frame;
			}

			ImGui::DragInt(std::string{ "Frame " + std::to_string(imguiCounter) }.c_str(), &keyframe.copyFrame, 1.f, 0, sequencer.data.lastFrame);

			if (keyframe.copyFrame != keyframe.frame) {
				// Verify if key is valid.. 
				if (
						keyframe.copyFrame < 0 || keyframe.copyFrame > sequencer.data.lastFrame
					||	std::ranges::find_if(sequencer.data.keyframes, [&](auto&& element) { return element.frame == keyframe.copyFrame; }) != sequencer.data.keyframes.end()
					) {
					// invalid..
					keyframe.copyFrame = keyframe.frame;
				}
				else {
					keyframe.frame = keyframe.copyFrame;
				}
			}

			ImGui::TableNextColumn();

			editor.displayEnumDropDownList<Sequencer::Keyframe::LerpType>(keyframe.lerpType, "Type", [&](auto value) {
				keyframe.lerpType = value;
				// force reanimate..
				sequence.lastTimeElapsed = -1.f;
			});

			ImGui::TableNextColumn();

			ImGui::InputFloat("Pow", &keyframe.power);

			if (ImGui::IsItemDeactivatedAfterEdit()) {
				// force reanimate..
				sequence.lastTimeElapsed = -1.f;
			}

			ImGui::TableNextColumn();

			if (ImGui::Button("[-]")) {
				it = sequencer.data.keyframes.erase(it);

				if (sequencer.data.keyframes.size()) {
					sequencer.data.lastFrame = sequencer.data.keyframes.back().frame;
				}
			}
			else {
				++it;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

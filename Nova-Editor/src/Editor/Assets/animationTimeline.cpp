#include "animationTimeline.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

#include "IconsFontAwesome6.h"
#include "imgui_neo_sequencer.h"

AnimationTimeLine::AnimationTimeLine(Editor& editor) :
	editor			{ editor },
	resourceManager { editor.resourceManager }
{}

void AnimationTimeLine::update() {
	ImGui::Begin(ICON_FA_TIMELINE " Sequencer");
	// ===================================================================================
	// We attempt to retrieve animation clip from the animator component of the currently selected entity.
	// ===================================================================================
	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	entt::entity selectedEntity = editor.getSelectedEntities()[0];
	
	Sequence* sequence = editor.engine.ecs.registry.try_get<Sequence>(selectedEntity);

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

	int currentFrame = 0;
	int startFrame = 0;
	int endFrame = (int) (sequencer->data.totalDuration * 30.f) + 1;

	ImGui::SliderFloat("Duration (in seconds)", &sequencer->data.totalDuration, 0.f, 100.f);

	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, {})) {
		ImGui::Button("[+]");

		if (ImGui::BeginNeoTimelineEx("Position", nullptr)) {
			ImGui::EndNeoTimeLine();
		}

		if (ImGui::BeginNeoTimelineEx("Scale", nullptr)) {
			ImGui::EndNeoTimeLine();
		}

		if (ImGui::BeginNeoTimelineEx("Rotation", nullptr)) {
			ImGui::EndNeoTimeLine();
		}

		ImGui::EndNeoSequencer();
	}

	ImGui::End();
}

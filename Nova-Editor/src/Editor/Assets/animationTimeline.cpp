#include "animationTimeline.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

#include "IconsFontAwesome6.h"

#include "imgui_neo_sequencer.h"

AnimationTimeLine::AnimationTimeLine(Editor& editor) :
	editor			{ editor },
	resourceManager { editor.resourceManager }
{
	ImGuiNeoSequencerStyle& style = ImGui::GetNeoSequencerStyle();
	auto& colors = style.Colors;

	colors[ImGuiNeoSequencerCol_Bg] = ImVec4(0.10f, 0.10f, 0.11f, 1.0f);
	colors[ImGuiNeoSequencerCol_TimelinesBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
	colors[ImGuiNeoSequencerCol_SelectedTimeline] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);

	colors[ImGuiNeoSequencerCol_TimelineBorder] = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
	colors[ImGuiNeoSequencerCol_TopBarBg] = ImVec4(0.13f, 0.13f, 0.15f, 1.0f);

	colors[ImGuiNeoSequencerCol_Selection] = ImVec4(0.30f, 0.50f, 0.80f, 0.60f);
	colors[ImGuiNeoSequencerCol_Keyframe] = ImVec4(0.80f, 0.80f, 0.85f, 1.0f);
	colors[ImGuiNeoSequencerCol_KeyframeHovered] = ImVec4(1.00f, 0.65f, 0.25f, 1.0f);
	colors[ImGuiNeoSequencerCol_KeyframeSelected] = ImVec4(1.00f, 0.45f, 0.10f, 1.0f);

	colors[ImGuiNeoSequencerCol_ZoomBarBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.0f);
	colors[ImGuiNeoSequencerCol_ZoomBarSlider] = ImVec4(0.30f, 0.10f, 0.11f, 1.0f);
	colors[ImGuiNeoSequencerCol_ZoomBarSliderHovered] = ImVec4(0.40f, 0.15f, 0.16f, 1.0f);

	colors[ImGuiNeoSequencerCol_ZoomBarSliderEnds] = ImVec4(0.03f, 0.03f, 0.20f, 1.0f);
	colors[ImGuiNeoSequencerCol_ZoomBarSliderEndsHovered] = ImVec4(0.05f, 0.05f, 0.25f, 1.0f);
}

void AnimationTimeLine::update() {
	ImGui::Begin(ICON_FA_TIMELINE " Animation");

	// ===================================================================================
	// We attempt to retrieve animation clip from the animator component of the currently selected entity.
	// ===================================================================================
	if (!editor.hasAnyEntitySelected()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	entt::entity selectedEntity = editor.getSelectedEntities()[0];
	
	Animator* animator = editor.engine.ecs.registry.try_get<Animator>(selectedEntity);

	if (!animator) {
		ImGui::Text("Selected entity has no animator component.");
		ImGui::End();
		return;
	}

	auto&& [model, loadStatus] = resourceManager.getResource<Model>(animator->modelId);

	if (!model) {
		switch (loadStatus)
		{
		case ResourceManager::QueryResult::Invalid:
			ImGui::Text("Animator is pointing to an invalid animation clip asset.");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::WrongType:
			ImGui::Text("This should never happened. Resource ID is not a model?");
			assert(false && "Resource ID is not a model.");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::Loading:
			ImGui::Text("Loading..");
			ImGui::End();
			return;
		case ResourceManager::QueryResult::LoadingFailed:
			ImGui::Text("Loading of model failed.");
			ImGui::End();
			return;
		default:
			assert(false);
			ImGui::End();
			return;
		}
	}

	// ===================================================================================
	// We display timeline accordingly to the animation clip's data..
	// ===================================================================================
	if (model->animations.empty()) {
		ImGui::Text("Animation vector is empty.");
		ImGui::End();
		return;
	}

	Animation const& animation = model->animations[0];

	int32_t currentFrame = 0;
	int32_t startFrame = 0;
	int32_t endFrame = 64;

	if (ImGui::BeginNeoSequencer(animation.name.c_str(), &currentFrame, &startFrame, &endFrame)) {
		std::vector<ImGui::FrameIndexType> frames;

		for (auto&& [boneName, channel] : animation.animationChannels) {
			// 1. position.
			if (channel.positions.size()) {
				if (ImGui::BeginNeoTimeline(std::string{ boneName + " - position" }.c_str(), frames)) {
					ImGui::EndNeoTimeLine();
				}
			}

			// 2. rotation.
			if (channel.rotations.size()) {
				if (ImGui::BeginNeoTimeline(std::string{ boneName + " - rotation" }.c_str(), frames)) {
					ImGui::EndNeoTimeLine();
				}
			}

			// 3. scale.
			if (channel.scalings.size()) {
				if (ImGui::BeginNeoTimeline(std::string{ boneName + " - scale" }.c_str(), frames)) {
					ImGui::EndNeoTimeLine();
				}
			}
		}

		ImGui::EndNeoSequencer();
	}

	ImGui::End();
}

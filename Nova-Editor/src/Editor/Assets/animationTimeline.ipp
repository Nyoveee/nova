#include "imgui_neo_sequencer.h"

template <typename T>
void AnimationTimeLine::displayTimeline(const char* name, std::vector<Sequencer::Keyframe<T>> const& keyframes) {
	if (ImGui::BeginNeoTimelineEx(name, nullptr)) {
		for (auto&& keyframe : keyframes) {
			int frame = keyframe.frame;

			ImGui::PushID(frame);
			ImGui::NeoKeyframe(&frame);
			ImGui::PopID();
		}

		ImGui::EndNeoTimeLine();
	}
}


template<typename T>
void AnimationTimeLine::displayKeyframes(Sequence& sequence, Sequencer& sequencer, std::vector<Sequencer::Keyframe<T>>& keyframes, const char* tableName) {
	if (ImGui::BeginTable(tableName, 4)) {
		ImGui::TableSetupColumn("Keyframe", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Lerp Type", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Power", ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, 40.0f);

		// Populate table rows
		int imguiCounter = -1;

		for (auto it = keyframes.begin(); it != keyframes.end();) {
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
					|| std::ranges::find_if(keyframes, [&](auto&& element) { return element.frame == keyframe.copyFrame; }) != keyframes.end()
				) {
					// invalid..
					keyframe.copyFrame = keyframe.frame;
				}
				else {
					keyframe.frame = keyframe.copyFrame;
				}
			}

			// we have to sort the whole keyframe array again in case of order change..
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				std::ranges::sort(keyframes, [&](auto&& lhs, auto&& rhs) {
					return lhs.frame < rhs.frame;
				});
			}

			ImGui::TableNextColumn();

			// lerp and power will not affect the 1st frame.
			if (imguiCounter == 0) {
				ImGui::BeginDisabled();
			}

			displayLerpEnumDropDownList(keyframe.lerpType, "Type", [&](typename Sequencer::LerpType value) {
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

			if (imguiCounter == 0) {
				ImGui::EndDisabled();
			}

			if (ImGui::Button("[-]")) {
				it = keyframes.erase(it);

				int newLastFrame = 0;

				// We need to appropriate determine the new last frame.
				for (auto&& positionKeyframe : sequencer.data.positionKeyframes) {
					newLastFrame = std::max(newLastFrame, positionKeyframe.frame);
				}

				for (auto&& rotationKeyframe : sequencer.data.rotationKeyframes) {
					newLastFrame = std::max(newLastFrame, rotationKeyframe.frame);
				}

				for (auto&& scaleKeyframe : sequencer.data.scaleKeyframes) {
					newLastFrame = std::max(newLastFrame, scaleKeyframe.frame);
				}

				sequencer.data.lastFrame = newLastFrame;
			}
			else {
				++it;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}
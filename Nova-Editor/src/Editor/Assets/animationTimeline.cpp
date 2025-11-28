#include "animationTimeline.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

#include "IconsFontAwesome6.h"

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
	selectedEntity			{ entt::null },
	isPlaying				{ false },
	editMode				{ EditMode::Transform }
{}

void AnimationTimeLine::update(float dt) {
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

	// if simulation is not active, we let edit dictate animation..
	if (!editor.engine.isInSimulationMode()) {
		// animate sequence..
		if (isPlaying) {
			sequence->timeElapsed += dt;

			if (sequence->currentFrame >= sequencer->data.lastFrame) {
				sequence->currentFrame = 0;
				sequence->timeElapsed = 0.f;
			}

			sequence->currentFrame = static_cast<int>(sequence->timeElapsed * FPS);
		}
		// update time elapsed based on the header position..
		else {
			sequence->currentFrame = currentFrame;
			sequence->timeElapsed = (float)currentFrame / FPS;
		}

	}

	ImGui::End();
}

void AnimationTimeLine::displayMainPanel(Sequence& sequence, Sequencer& sequencer) {
	displayTopPanel(sequencer);

	int startFrame = 0;
	int endFrame = static_cast<int>(viewDuration * FPS);

	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, {}, ImGuiNeoSequencerFlags_Selection_DisplayTimeInsteadOfFrames)) {
		if (ImGui::BeginNeoTimelineEx("Event", nullptr, ImGuiNeoTimelineFlags_Group)) {
			for (auto&& animationEvent : sequencer.data.animationEvents) {
				int frame = animationEvent.key;

				ImGui::PushID(frame);
				ImGui::NeoKeyframe(&frame);
				ImGui::PopID();
			}

			ImGui::EndNeoTimeLine();
		}
	
		int counter = 0;

		ImGui::PushID(++counter);
		displayTimeline("Position", sequencer.data.positionKeyframes);
		ImGui::PopID();

		ImGui::PushID(++counter);
		displayTimeline("Rotation", sequencer.data.rotationKeyframes);
		ImGui::PopID();

		ImGui::PushID(++counter);
		displayTimeline("Scale", sequencer.data.scaleKeyframes);
		ImGui::PopID();

		ImGui::EndNeoSequencer();
	}

	if (ImGui::BeginTabBar("Sequencer Tab Bar")) {
		if (ImGui::BeginTabItem("Keyframes")) {
			displayAllKeyframes(sequence, sequencer);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Animation Events")) {
			displayAnimationEvents(sequence, sequencer);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void AnimationTimeLine::displayTopPanel(Sequencer& sequencer) {
	if (editor.engine.isInSimulationMode()) {
		ImGui::BeginDisabled();
	}

	displayTopToolbar(sequencer);

	if (editor.engine.isInSimulationMode()) {
		ImGui::EndDisabled();
	}
}

void AnimationTimeLine::displayTopToolbar(Sequencer& sequencer) {
	constexpr float toolbarButtonSize = 30.f;
	constexpr float toolbarInputSize = 45.f;
	constexpr float viewSliderSize = 200.f;
	constexpr float padding = 50.f;
	constexpr float editModeDropDownList = 200.f;

	// I want to center the top toolbar. Therefore, I need to calculate the total width of the table.
	// Its defined by the sizes of my buttons, paddings and input fields.
	float windowWidth = 7 * toolbarButtonSize + 5 * padding + 2 * toolbarInputSize + viewSliderSize + editModeDropDownList;
	float offsetX = (ImGui::GetContentRegionAvail().x - windowWidth) / 2.f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

	ImGui::PushStyleVarY(ImGuiStyleVar_WindowPadding, 0.f);

	ImGui::BeginChild("Top Toolbar", { windowWidth , 0.f }, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	// Display the whole controls button overlay..
	if (ImGui::BeginTable("Control", 11)) {
		// Set up frame columns..
		ImGui::TableSetupColumn("Start Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Prev Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Play", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Next Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("End Frame", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Add Keyframe", ImGuiTableColumnFlags_WidthFixed, padding + toolbarButtonSize);
		ImGui::TableSetupColumn("Add Animation event", ImGuiTableColumnFlags_WidthFixed, toolbarButtonSize);
		ImGui::TableSetupColumn("Current Frame", ImGuiTableColumnFlags_WidthFixed, toolbarInputSize + padding);
		ImGui::TableSetupColumn("View Slider", ImGuiTableColumnFlags_WidthFixed, viewSliderSize + 2 * padding);
		ImGui::TableSetupColumn("Last Frame", ImGuiTableColumnFlags_WidthFixed, toolbarInputSize + padding);
		ImGui::TableSetupColumn("Edit Mode", ImGuiTableColumnFlags_WidthFixed, editModeDropDownList);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_BACKWARD_FAST)) {
#if false
			if(sequencer.data.keyframes.size())
				currentFrame = sequencer.data.keyframes.front().frame;
#endif
		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_BACKWARD_STEP)) {
#if false
			for (auto reverseIt = sequencer.data.keyframes.rbegin(); reverseIt != sequencer.data.keyframes.rend(); ++reverseIt) {
				if (reverseIt->frame < currentFrame) {
					currentFrame = reverseIt->frame;
					break;
				}
			}
#endif
		}

		ImGui::TableNextColumn();
		if (ImGui::Button(isPlaying ? ICON_FA_PAUSE : ICON_FA_PLAY)) {
			isPlaying = !isPlaying;
		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_FORWARD_STEP)) {
#if false
			for (auto&& keyframe : sequencer.data.keyframes) {
				if (keyframe.frame > currentFrame) {
					currentFrame = keyframe.frame;
					break;
				}
			}
#endif
		}

		ImGui::TableNextColumn();
		if (ImGui::Button(ICON_FA_FORWARD_FAST)) {
#if false
			if(sequencer.data.keyframes.size())
				currentFrame = sequencer.data.keyframes.back().frame;
#endif
		}

		ImGui::TableNextColumn();
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
		if (ImGui::Button((std::string{ "+" } + ICON_FA_DIAMOND).c_str())) {
			Transform const& transform = registry.get<Transform>(selectedEntity);

			switch (editMode)
			{
			case AnimationTimeLine::EditMode::Transform:
				sequencer.recordKeyframe<glm::vec3>(currentFrame, transform.localPosition, sequencer.data.positionKeyframes);
				sequencer.recordKeyframe<glm::quat>(currentFrame, transform.localRotation, sequencer.data.rotationKeyframes);
				sequencer.recordKeyframe<glm::vec3>(currentFrame, transform.localScale, sequencer.data.scaleKeyframes);
				break;
			case AnimationTimeLine::EditMode::Position:
				sequencer.recordKeyframe<glm::vec3>(currentFrame, transform.localPosition, sequencer.data.positionKeyframes);
				break;
			case AnimationTimeLine::EditMode::Rotation:
				sequencer.recordKeyframe<glm::quat>(currentFrame, transform.localRotation, sequencer.data.rotationKeyframes);
				break;
			case AnimationTimeLine::EditMode::Scale:
				sequencer.recordKeyframe<glm::vec3>(currentFrame, transform.localScale, sequencer.data.scaleKeyframes);
				break;
			}
		}

		ImGui::TableNextColumn();
		if (ImGui::Button((std::string{ "+" } + ICON_FA_BOOKMARK).c_str())) {
			// current frame do not already have an animation event..
			if (std::ranges::find_if(sequencer.data.animationEvents, [&](auto&& element) { return element.key == currentFrame; }) == sequencer.data.animationEvents.end()) {
				sequencer.data.animationEvents.push_back({ currentFrame, TypedResourceID<ScriptAsset>{ INVALID_RESOURCE_ID }, "event", -1 });
			}
		}

		ImGui::TableNextColumn();

		// offset..
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
		ImGui::SetNextItemWidth(toolbarInputSize);
		ImGui::InputInt("##", &currentFrame, 0, 0);

		float lastFrameInSeconds = (float)(sequencer.data.lastFrame) / (float)FPS;

		if (viewDuration < lastFrameInSeconds) {
			viewDuration = lastFrameInSeconds;
		}

		ImGui::TableNextColumn();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
		ImGui::SetNextItemWidth(viewSliderSize);
		ImGui::DragFloat("##View (in seconds)", &viewDuration, 1.f, lastFrameInSeconds, HUNDRED_SECONDS);

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(toolbarInputSize);
		ImGui::InputInt("##Last Frame", &sequencer.data.lastFrame, 0, 0);

		ImGui::TableNextColumn();
		editor.displayEnumDropDownList<EditMode>(editMode, "##Edit Mode", [&](EditMode newMode) { editMode = newMode; });

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();

	ImGui::EndChild();

	ImGui::PopStyleVar();
}

void AnimationTimeLine::displayAllKeyframes(Sequence& sequence, Sequencer& sequencer) {
	if (ImGui::BeginTabBar("Key frames tab bar")) {
		if (ImGui::BeginTabItem("Position")) {
			displayKeyframes<glm::vec3>(sequence, sequencer, sequencer.data.positionKeyframes, "Position Table");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Rotation")) {
			displayKeyframes<glm::quat>(sequence, sequencer, sequencer.data.rotationKeyframes, "Rotation Table");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Scale")) {
			displayKeyframes<glm::vec3>(sequence, sequencer, sequencer.data.scaleKeyframes, "Scale Table");
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void AnimationTimeLine::displayAnimationEvents([[maybe_unused]] Sequence& sequence, Sequencer& sequencer) {
	if (ImGui::BeginTable("Animation Event Table", 4)) {
		ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Script", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, 40.0f);

		// Populate table rows
		int imguiCounter = 0;
		
		for (auto it = sequencer.data.animationEvents.begin(); it != sequencer.data.animationEvents.end();) {
			auto&& animationEvent = *it;
			auto&& [key, scriptId, name, copyKey] = animationEvent;

			ImGui::PushID(imguiCounter++);

			ImGui::TableNextRow();		// Start a new row
			
			ImGui::TableSetColumnIndex(0);
			
			if (copyKey == -1) {
				copyKey = key;
			}

			ImGui::DragInt("Key", &copyKey);

			// Verify if key is valid..
			if (
					copyKey < 0 || copyKey > sequencer.data.lastFrame
				||  std::ranges::find_if(sequencer.data.animationEvents, [&](auto&& event) { return event.key == copyKey; }) != sequencer.data.animationEvents.end()
			) {
				copyKey = key;
			}
			else {
				key = copyKey;
			}

			ImGui::TableSetColumnIndex(1);
			editor.displayEntityScriptDropDownList(scriptId, "##script", selectedEntity, [&](ResourceID newScriptId) {
				scriptId = TypedResourceID<ScriptAsset>{ newScriptId };
			});

			ImGui::TableSetColumnIndex(2);
			ImGui::InputText("Function name", &animationEvent.functionName);

			ImGui::TableSetColumnIndex(3);
	
			if (ImGui::Button("[-]")) {
				it = sequencer.data.animationEvents.erase(it);
			}
			else {
				++it;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void AnimationTimeLine::displayLerpEnumDropDownList(typename Sequencer::LerpType value, const char* labelName, std::function<void(typename Sequencer::LerpType)> onClickCallback) {
	editor.displayEnumDropDownList(value, labelName, onClickCallback);
}

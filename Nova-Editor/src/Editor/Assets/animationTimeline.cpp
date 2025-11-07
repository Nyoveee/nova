#include "animationTimeline.h"

#include "Editor/editor.h"
#include "Engine/engine.h"
#include "ResourceManager/resourceManager.h"

#include "IconsFontAwesome6.h"


AnimationTimeLine::AnimationTimeLine(Editor& editor) :
	editor			{ editor },
	resourceManager { editor.resourceManager }
{}

void AnimationTimeLine::update() {
#if 0
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

	
	}

	ImGui::End();
#endif
}

#pragma once

#include <entt/entt.hpp>

#include "component.h"
#include "resource.h"

class Editor;
class ResourceManager;
class Sequencer;

struct Sequence;

class AnimationTimeLine {
public:
	enum class EditMode {
		Transform,
		Position,
		Rotation,
		Scale
	};
public:
	AnimationTimeLine(Editor& editor);
	
public:
	void update(float dt);

private:
	void displayMainPanel(Sequence& sequence, Sequencer& sequencer);

	void displayTopPanel(Sequencer& sequencer);

	void displayTopToolbar(Sequencer& sequencer);

	void displayAllKeyframes(Sequence& sequence, Sequencer& sequencer);
	void displayAnimationEvents(Sequence& sequence, Sequencer& sequencer);

	template <typename T>
	void displayTimeline(const char* name, std::vector<Sequencer::Keyframe<T>> const& keyframes);

	template <typename T>
	void displayKeyframes(Sequence& sequence, Sequencer& sequencer, std::vector<Sequencer::Keyframe<T>>& keyframes, const char * tableName);

	void displayLerpEnumDropDownList(typename Sequencer::LerpType value, const char* labelName, std::function<void(typename Sequencer::LerpType)> onClickCallback);

private:
	Editor& editor;
	ResourceManager& resourceManager;
	entt::registry& registry;

	float viewDuration;
	int currentFrame;
	entt::entity selectedEntity;

	bool isPlaying;

	EditMode editMode;
};

#include "animationTimeline.ipp"

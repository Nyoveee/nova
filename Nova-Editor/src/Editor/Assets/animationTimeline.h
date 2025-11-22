#pragma once

#include <entt/entt.hpp>

class Editor;
class ResourceManager;
class Sequencer;

struct Sequence;

class AnimationTimeLine {
public:
	AnimationTimeLine(Editor& editor);
	
public:
	void update(float dt);

private:
	void displayMainPanel(Sequence& sequence, Sequencer& sequencer);

	void displayTopPanel(Sequencer& sequencer);

	void displayTopToolbar(Sequencer& sequencer);

	void displayKeyframes(Sequence& sequence, Sequencer& sequencer);
	void displayAnimationEvents(Sequence& sequence, Sequencer& sequencer);

private:
	Editor& editor;
	ResourceManager& resourceManager;
	entt::registry& registry;

	float viewDuration;
	int currentFrame;
	entt::entity selectedEntity;

	bool isPlaying;
};
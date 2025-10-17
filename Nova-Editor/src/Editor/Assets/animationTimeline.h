#pragma once

class Editor;
class ResourceManager;

class AnimationTimeLine {
public:
	AnimationTimeLine(Editor& editor);
	
public:
	void update();

private:
	Editor& editor;
	ResourceManager& resourceManager;
};
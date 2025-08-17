#pragma once

#include <memory>

class Editor;
class ECS;

class ComponentInspector {
public:
	ComponentInspector(Editor& editor, ECS& ecs);

public:
	void update();

public:
	ECS& ecs;
	Editor& editor;
};
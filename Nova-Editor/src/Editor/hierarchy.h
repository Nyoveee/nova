#pragma once

#include <entt/entt.hpp>

class ECS;
class Editor;

class Hierarchy {
public:
	Hierarchy(Editor& editor);

public:
	void update();

private:
	void createGameObject();
	void displayEntityHierarchy(entt::entity entity);

private:
	ECS& ecs;
	Editor& editor;
};
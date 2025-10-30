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
	void createGameObject(entt::registry& registry);
	void displayEntityHierarchy(entt::entity entity, entt::registry& registry);
	void displayEntityWindow();
	void displayUIWindow();

private:
	ECS& ecs;
	Editor& editor;
};
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
	void displayHierarchyWindow();
	void displayLayerTable();
	void displayAllEntityIds(const char* labelName, entt::entity selectedEntity, std::function<void(entt::entity)> const& onClickCallback);

public:
	bool isHovering = false;

private:
	ECS& ecs;
	Editor& editor;

	std::string searchQuery;
	std::string uppercaseSearchQuery;
	std::string uppercaseEntityName;

	std::string entitySearchIdQuery;
};
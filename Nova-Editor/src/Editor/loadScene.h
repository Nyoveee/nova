#pragma once

#include <entt/entt.hpp>

class ECS;
class Editor;

class LoadScene {
	public:
		LoadScene(Editor& editor);

	private:
		ECS& ecs;
		Editor& editor;

};
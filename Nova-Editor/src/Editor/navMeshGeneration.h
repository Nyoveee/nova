#pragma once

#include <entt/entt.hpp>


class ECS;
class Editor;

class navMeshGeneration
{

public:
	navMeshGeneration(Editor& editor);

	void SetAgent();

	void BuildNavMesh();

private:
	ECS& ecs;
	Editor& editor;




};


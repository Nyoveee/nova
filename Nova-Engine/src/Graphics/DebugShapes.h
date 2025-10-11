#pragma once
#include "vertex.h"
#include "component.h"

#include <vector>
namespace DebugShapes
{
	std::vector<SimpleVertex> SphereAxisXY(Transform const& transform, float radius);
	std::vector<SimpleVertex> SphereAxisXZ(Transform const& transform, float radius);
	std::vector<SimpleVertex> SphereAxisYZ(Transform const& transform, float radius);
	std::vector<SimpleVertex> HemisphereAxisXY(Transform const& transform, float radius);
	std::vector<SimpleVertex> HemisphereAxisYZ(Transform const& transform, float radius);
	std::vector<SimpleVertex> Cube(Transform const& transform, glm::vec3 min, glm::vec3 max);
	std::vector<SimpleVertex> ConeOuterAxisXZ(Transform const& transform, float radius, float arc, float distance);
	std::vector<SimpleVertex> ConeEdges(Transform const& transform, float radius, float arc, float distance);
	std::vector<SimpleVertex> Edge(Transform const& transform, float distance);
	constexpr int NUM_DEBUG_CIRCLE_POINTS = 36;
};


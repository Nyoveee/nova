#pragma once
#include "vertex.h"
#include "component.h"

#include <vector>
namespace DebugShapes
{
	std::vector<SimpleVertex> SphereAxisXY(float radius);
	std::vector<SimpleVertex> SphereAxisXZ(float radius);
	std::vector<SimpleVertex> SphereAxisYZ(float radius);
	std::vector<SimpleVertex> HemisphereAxisXY(float radius);
	std::vector<SimpleVertex> HemisphereAxisYZ(float radius);
	std::vector<SimpleVertex> Cube(glm::vec3 min, glm::vec3 max);
	std::vector<SimpleVertex> ConeOuterAxisXZ(float radius, float arc, float distance);
	std::vector<SimpleVertex> ConeEdges(float radius, float arc, float distance);
	std::vector<SimpleVertex> Edge(float distance);
	constexpr int NUM_DEBUG_CIRCLE_POINTS = 36;
};


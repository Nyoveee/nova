#pragma once
#include "vertex.h"
#include "component.h"

#include <vector>

class Camera;

namespace DebugShapes
{
	std::vector<glm::vec3> SphereAxisXY(float radius);
	std::vector<glm::vec3> SphereAxisXZ(float radius);
	std::vector<glm::vec3> SphereAxisYZ(float radius);
	std::vector<glm::vec3> HemisphereAxisXY(float radius);
	std::vector<glm::vec3> HemisphereAxisYZ(float radius);
	
	std::vector<glm::vec3> Cube(glm::vec3 const& min, glm::vec3 const& max);
	std::vector<glm::vec3> Cube(AABB const& aabb);

	std::vector<glm::vec3> ConeOuterAxisXZ(float radius, float arc, float distance);
	std::vector<glm::vec3> ConeEdges(float radius, float arc, float distance);
	std::vector<glm::vec3> Edge(float distance);

	std::vector<glm::vec3> CameraFrustumOutline(glm::vec3 position, Camera const& camera);
	std::vector<glm::vec3> CameraFrustum(glm::vec3 position, Camera const& camera);

	constexpr int NUM_DEBUG_CIRCLE_POINTS = 36;
};


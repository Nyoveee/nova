#include "DebugShapes.h"
#include "type_alias.h"
#include <numbers>
namespace{
	constexpr float PI = 2 * std::numbers::pi_v<float>;
	constexpr float PI2 = 2 * std::numbers::pi_v<float>;
}
std::vector<glm::vec3> DebugShapes::SphereAxisXY(float radius)
{
	std::vector<glm::vec3> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS) {
		result.push_back(glm::vec3{ std::cos(i),std::sin(i),0.f } * radius);
	}
		
	return result;
}

std::vector<glm::vec3> DebugShapes::SphereAxisXZ(float radius)
{
	std::vector<glm::vec3> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ std::cos(i),0.f,std::sin(i) });
	return result;
}

std::vector<glm::vec3> DebugShapes::SphereAxisYZ(float radius)
{
	std::vector<glm::vec3> result;

	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ 0.f,std::cos(i),std::sin(i) } * radius);
	return result;
}

std::vector<glm::vec3> DebugShapes::HemisphereAxisXY(float radius)
{
	std::vector<glm::vec3> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ std::cos(i),std::sin(i),0.f } * radius);
	return result;
}

std::vector<glm::vec3> DebugShapes::HemisphereAxisYZ(float radius)
{
	std::vector<glm::vec3> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ 0.f,std::abs(std::sin(i)),std::cos(i)} * radius);
	return result;
}

std::vector<glm::vec3> DebugShapes::Cube(glm::vec3 min, glm::vec3 max)
{
	std::vector<glm::vec3> result;
	// Front
	result.push_back(min);
	result.push_back(glm::vec3{max.x,min.y,min.z});
	result.push_back(min);
	result.push_back(glm::vec3{min.x,max.y,min.z});
	result.push_back(glm::vec3{min.x,max.y,min.z});
	result.push_back(glm::vec3{max.x,max.y,min.z});
	result.push_back(glm::vec3{max.x,min.y,min.z});
	result.push_back(glm::vec3{max.x,max.y,min.z});

	// Back
	result.push_back(glm::vec3{min.x,min.y,max.z});
	result.push_back(glm::vec3{max.x,min.y,max.z});
	result.push_back(glm::vec3{min.x,min.y,max.z});
	result.push_back(glm::vec3{min.x,max.y,max.z});
	result.push_back(glm::vec3{min.x,max.y,max.z});
	result.push_back(max);
	result.push_back(glm::vec3{max.x,min.y,max.z});
	result.push_back(max);

	// Sides
	result.push_back(min);
	result.push_back(glm::vec3{min.x,min.y,max.z});
	result.push_back(glm::vec3{min.x,max.y,min.z});
	result.push_back(glm::vec3{min.x,max.y,max.z});
	result.push_back(glm::vec3{max.x,min.y,min.z});
	result.push_back(glm::vec3{max.x,min.y,max.z});
	result.push_back(glm::vec3{max.x,max.y,min.z});
	result.push_back(max);
	return result;
}

std::vector<glm::vec3> DebugShapes::ConeOuterAxisXZ(float radius, float arc, float distance)
{
	std::vector<glm::vec3> result;
	arc = Radian{ Degree{std::clamp(arc, 0.f, 75.f)} };
	radius += distance * std::sin(arc);
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ std::cos(i), 0, std::sin(i) } * radius + glm::vec3{0,distance,0});
	return result;
}

std::vector<glm::vec3> DebugShapes::ConeEdges(float radius, float arc, float distance)
{
	std::vector<glm::vec3> result;
	arc = Radian{ Degree{std::clamp(arc, 0.f, 75.f)} };
	float outerRadius = radius + distance * std::sin(arc);
	result.push_back(glm::vec3{radius,0,0});
	result.push_back(glm::vec3{outerRadius,distance,0});
	result.push_back(glm::vec3{-radius,0,0});
	result.push_back(glm::vec3{-outerRadius,distance,0});
	result.push_back(glm::vec3{0,0,radius});
	result.push_back(glm::vec3{0,distance, outerRadius});
	result.push_back(glm::vec3{0,0,-radius});
	result.push_back(glm::vec3{0,distance, -outerRadius});

	return result;
}

std::vector<glm::vec3> DebugShapes::Edge(float distance)
{
	std::vector<glm::vec3> result;
	result.push_back(glm::vec3{ 1,0,0 } * distance / 2.f);
	result.push_back(-glm::vec3{ 1,0,0 } * distance / 2.f);
	return result;
}

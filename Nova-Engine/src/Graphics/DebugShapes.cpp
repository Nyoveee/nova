#include "DebugShapes.h"
#include "type_alias.h"
#include <numbers>
namespace{
	constexpr float PI = 2 * std::numbers::pi_v<float>;
	constexpr float PI2 = 2 * std::numbers::pi_v<float>;
}
std::vector<SimpleVertex> DebugShapes::SphereAxisXY(float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS) {
		result.push_back(SimpleVertex{ glm::vec3{ std::cos(i),std::sin(i),0.f } *radius });
	}
		
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisXZ(float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{  glm::vec3{ std::cos(i),0.f,std::sin(i) } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisYZ(float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ glm::vec3{ 0.f,std::cos(i),std::sin(i) } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::HemisphereAxisXY(float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{glm::vec3{ std::cos(i),std::sin(i),0.f } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::HemisphereAxisYZ(float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ glm::vec3{ 0.f,std::abs(std::sin(i)),std::cos(i)} *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::Cube(glm::vec3 min, glm::vec3 max)
{
	std::vector<SimpleVertex> result;
	// Front
	result.push_back(SimpleVertex{ min });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ min });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,max.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,max.y,min.z} });
	// Back
	result.push_back(SimpleVertex{ glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ max });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ max });
	// Sides
	result.push_back(SimpleVertex{ min });
	result.push_back(SimpleVertex{ glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ glm::vec3{max.x,max.y,min.z} });
	result.push_back(SimpleVertex{ max });
	return result;
}

std::vector<SimpleVertex> DebugShapes::ConeOuterAxisXZ(float radius, float arc, float distance)
{
	std::vector<SimpleVertex> result;
	arc = Radian{ Degree{std::clamp(arc, 0.f, 75.f)} };
	radius += distance * std::sin(arc);
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ glm::vec3{ std::cos(i), 0, std::sin(i) } * radius + glm::vec3{0,distance,0} });
	return result;
}

std::vector<SimpleVertex> DebugShapes::ConeEdges(float radius, float arc, float distance)
{
	std::vector<SimpleVertex> result;
	arc = Radian{ Degree{std::clamp(arc, 0.f, 75.f)} };
	float outerRadius = radius + distance * std::sin(arc);
	result.push_back(SimpleVertex{ glm::vec3{radius,0,0} });
	result.push_back(SimpleVertex{ glm::vec3{outerRadius,distance,0} });
	result.push_back(SimpleVertex{ glm::vec3{-radius,0,0} });
	result.push_back(SimpleVertex{ glm::vec3{-outerRadius,distance,0} });
	result.push_back(SimpleVertex{ glm::vec3{0,0,radius} });
	result.push_back(SimpleVertex{ glm::vec3{0,distance, outerRadius} });
	result.push_back(SimpleVertex{ glm::vec3{0,0,-radius} });
	result.push_back(SimpleVertex{ glm::vec3{0,distance, -outerRadius} });
	return result;
}

std::vector<SimpleVertex> DebugShapes::Edge(float distance)
{
	std::vector<SimpleVertex> result;
	result.push_back(SimpleVertex{ glm::vec3{ 1,0,0 } * distance / 2.f });
	result.push_back(SimpleVertex{ -glm::vec3{ 1,0,0 } * distance / 2.f });
	return result;
}

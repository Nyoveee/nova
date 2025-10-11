#include "DebugShapes.h"
#include <numbers>
namespace{
	constexpr float PI = 2 * std::numbers::pi_v<float>;
	constexpr float PI2 = 2 * std::numbers::pi_v<float>;
}
std::vector<SimpleVertex> DebugShapes::SphereAxisXY(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ transform.position + glm::vec3{ std::cos(i),std::sin(i),0.f } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisXZ(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ transform.position + glm::vec3{ std::cos(i),0.f,std::sin(i) } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisYZ(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ transform.position + glm::vec3{ 0.f,std::cos(i),std::sin(i) } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::HemisphereAxisXY(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ transform.position + glm::vec3{ std::cos(i),std::sin(i),0.f } *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::HemisphereAxisYZ(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i <= PI; i += PI / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(SimpleVertex{ transform.position + glm::vec3{ 0.f,std::abs(std::sin(i)),std::cos(i)} *radius });
	return result;
}

std::vector<SimpleVertex> DebugShapes::Cube(Transform const& transform, glm::vec3 min, glm::vec3 max)
{
	std::vector<SimpleVertex> result;
	// Front
	result.push_back(SimpleVertex{ transform.position + min });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + min });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,max.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,max.y,min.z} });
	// Back
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + max });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + max });
	// Sides
	result.push_back(SimpleVertex{ transform.position + min });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{min.x,max.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,min.y,max.z} });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{max.x,max.y,min.z} });
	result.push_back(SimpleVertex{ transform.position + max });
	return result;
}

std::vector<SimpleVertex> DebugShapes::Edge(Transform const& transform, float distance)
{
	std::vector<SimpleVertex> result;
	result.push_back(SimpleVertex{ transform.position - glm::vec3{ 1,0,0 } * distance / 2.f });
	result.push_back(SimpleVertex{ transform.position + glm::vec3{ 1,0,0 } * distance / 2.f });
	return result;
}

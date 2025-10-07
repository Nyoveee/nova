#include "DebugShapes.h"
#include <numbers>
namespace{
	constexpr float _2PI = 2 * std::numbers::pi_v<float>;
}
std::vector<SimpleVertex> DebugShapes::SphereAxisXY(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < _2PI; i += _2PI / NUM_DEBUG_CIRCLE_POINTS) {
		SimpleVertex vertex{ transform.position + glm::vec3{ std::cos(i),std::sin(i),0.f } * radius};
		result.push_back(vertex);
	}
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisXZ(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < _2PI; i += _2PI / NUM_DEBUG_CIRCLE_POINTS) {
		SimpleVertex vertex{ transform.position + glm::vec3{ std::cos(i),0.f,std::sin(i) } * radius};
		result.push_back(vertex);
	}
	return result;
}

std::vector<SimpleVertex> DebugShapes::SphereAxisYZ(Transform const& transform, float radius)
{
	std::vector<SimpleVertex> result;
	for (float i{}; i < _2PI; i += _2PI / NUM_DEBUG_CIRCLE_POINTS) {
		SimpleVertex vertex{ transform.position + glm::vec3{ 0.f,std::cos(i),std::sin(i) } * radius };
		result.push_back(vertex);
	}
	return result;
}

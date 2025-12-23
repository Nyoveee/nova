#include "DebugShapes.h"
#include "type_alias.h"
#include "camera.h"

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

std::vector<glm::vec3> DebugShapes::SphereAxisXZ([[maybe_unused]] float radius)
{
	std::vector<glm::vec3> result;
	for (float i{}; i < PI2; i += PI2 / NUM_DEBUG_CIRCLE_POINTS)
		result.push_back(glm::vec3{ std::cos(i),0.f,std::sin(i) } * radius);
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

std::vector<glm::vec3> DebugShapes::Cube(glm::vec3 const& min, glm::vec3 const& max)
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

std::vector<glm::vec3> DebugShapes::Cube(AABB const& aabb)
{
	return Cube(aabb.center - aabb.extent, aabb.center + aabb.extent);
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

std::vector<glm::vec3> DebugShapes::Line(glm::vec3 start, glm::vec3 end) {
	std::vector<glm::vec3> result;
	result.push_back(start);
	result.push_back(end);
	return result;
}

#if false
std::vector<glm::vec3> DebugShapes::Triangle(float size, Radian rotation, glm::vec3 offset) {
	std::vector<glm::vec3> result;

	Radian sineAngle	= std::sin(std::numbers::pi_v<float> / 2.f + rotation);
	Radian cosAngle		= std::cos(std::numbers::pi_v<float> / 2.f + rotation);

	result.push_back(glm::vec3{ size * std::cos(rotation), size * std::sin(rotation), 0.f} + offset);
	result.push_back(glm::vec3{ size * -cosAngle, sineAngle, 0.f } + offset);
	result.push_back(glm::vec3{ size * -cosAngle, -sineAngle, 0.f } + offset);

	return result;
}
#endif

std::vector<glm::vec3> DebugShapes::CameraFrustumOutline(glm::vec3 position, Camera const& camera) {
	// @TODO: Use a index buffer?

	std::vector<glm::vec3> frustumDebugPoints;

	// We calculate the world position of these frustum points.
	// -------------------------------------------------------
	// 1.1 Near plane points
	
	glm::vec3 centerOfNearPlane = position + camera.getFront() * camera.getNearPlaneDistance();

	// Using vertical fov angle, we can calculate the near plane horizontal and vertical length.
	float halfNearPlaneVerticalLength = camera.getNearPlaneDistance() * std::tan(camera.getFov() * .5f);
	float halfNearPlaneHorizontalLength = halfNearPlaneVerticalLength * camera.getAspectRatio();

	glm::vec3 topLeftNearPlane = centerOfNearPlane
		- camera.getRight() * halfNearPlaneHorizontalLength	// left offset
		+ camera.getUp() * halfNearPlaneVerticalLength;		// up offset

	glm::vec3 topRightNearPlane = centerOfNearPlane
		+ camera.getRight() * halfNearPlaneHorizontalLength	// right offset
		+ camera.getUp() * halfNearPlaneVerticalLength;		// up offset

	glm::vec3 bottomLeftNearPlane = centerOfNearPlane
		- camera.getRight() * halfNearPlaneHorizontalLength	// left offset
		- camera.getUp() * halfNearPlaneVerticalLength;		// down offset

	glm::vec3 bottomRightNearPlane = centerOfNearPlane
		+ camera.getRight() * halfNearPlaneHorizontalLength	// right offset
		- camera.getUp() * halfNearPlaneVerticalLength;		// down offset

	// -------------------------------------------------------
	// 1.2 Far plane points
	
	glm::vec3 centerOfFarPlane = position + camera.getFront() * camera.getFarPlaneDistance();
	
	// Using vertical fov angle, we can calculate the far plane horizontal and vertical length.
	float halfFarPlaneVerticalLength = camera.getFarPlaneDistance() * std::tan(camera.getFov() * .5f);
	float halfFarPlaneHorizontalLength = halfFarPlaneVerticalLength * camera.getAspectRatio();

	glm::vec3 topLeftFarPlane = centerOfFarPlane
		- camera.getRight() * halfFarPlaneHorizontalLength	// left offset
		+ camera.getUp() * halfFarPlaneVerticalLength;		// up offset

	glm::vec3 topRightFarPlane = centerOfFarPlane
		+ camera.getRight() * halfFarPlaneHorizontalLength	// right offset
		+ camera.getUp() * halfFarPlaneVerticalLength;		// up offset

	glm::vec3 bottomLeftFarPlane = centerOfFarPlane
		- camera.getRight() * halfFarPlaneHorizontalLength	// left offset
		- camera.getUp() * halfFarPlaneVerticalLength;		// down offset

	glm::vec3 bottomRightFarPlane = centerOfFarPlane
		+ camera.getRight() * halfFarPlaneHorizontalLength	// right offset
		- camera.getUp() * halfFarPlaneVerticalLength;		// down offset

	// -------------------------------------------------------
	// 2. Lines visualising the frustum. (Keep in mind we are using GL_LINE, cause a frustum is inherently disconnected, and I rather do it in one draw call)
	// (Near plane) Top horizontal line.
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(topRightNearPlane);
	
	// (Near plane) Bottom horizontal line.
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);

	// (Near plane) Left vertical line.
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);

	// (Near plane) Right vertical line.
	frustumDebugPoints.push_back(topRightNearPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);

	// (Far plane) Top horizontal line.
	frustumDebugPoints.push_back(topLeftFarPlane);
	frustumDebugPoints.push_back(topRightFarPlane);

	// (Far plane) Bottom horizontal line.
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Far plane) Left vertical line.
	frustumDebugPoints.push_back(topLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);

	// (Far plane) Right vertical line.
	frustumDebugPoints.push_back(topRightFarPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Left plane) Top line.
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(topLeftFarPlane);

	// (Left plane) Bottom line.
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	
	// (Right plane) Top line.
	frustumDebugPoints.push_back(topRightNearPlane);
	frustumDebugPoints.push_back(topRightFarPlane);

	// (Right plane) Bottom line.
	frustumDebugPoints.push_back(bottomRightNearPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	return frustumDebugPoints;
}

std::vector<glm::vec3> DebugShapes::CameraFrustum(glm::vec3 position, Camera const& camera) {
	std::vector<glm::vec3> frustumDebugPoints;

	// We calculate the world position of these frustum points.
	// -------------------------------------------------------
	// 1.1 Near plane points

	glm::vec3 centerOfNearPlane = position + camera.getFront() * camera.getNearPlaneDistance();

	// Using vertical fov angle, we can calculate the near plane horizontal and vertical length.
	float halfNearPlaneVerticalLength = camera.getNearPlaneDistance() * std::tan(camera.getFov() * .5f);
	float halfNearPlaneHorizontalLength = halfNearPlaneVerticalLength * camera.getAspectRatio();

	glm::vec3 topLeftNearPlane = centerOfNearPlane
		- camera.getRight() * halfNearPlaneHorizontalLength	// left offset
		+ camera.getUp() * halfNearPlaneVerticalLength;		// up offset

	glm::vec3 topRightNearPlane = centerOfNearPlane
		+ camera.getRight() * halfNearPlaneHorizontalLength	// right offset
		+ camera.getUp() * halfNearPlaneVerticalLength;		// up offset

	glm::vec3 bottomLeftNearPlane = centerOfNearPlane
		- camera.getRight() * halfNearPlaneHorizontalLength	// left offset
		- camera.getUp() * halfNearPlaneVerticalLength;		// down offset

	glm::vec3 bottomRightNearPlane = centerOfNearPlane
		+ camera.getRight() * halfNearPlaneHorizontalLength	// right offset
		- camera.getUp() * halfNearPlaneVerticalLength;		// down offset

	// -------------------------------------------------------
	// 1.2 Far plane points

	glm::vec3 centerOfFarPlane = position + camera.getFront() * camera.getFarPlaneDistance();

	// Using vertical fov angle, we can calculate the far plane horizontal and vertical length.
	float halfFarPlaneVerticalLength = camera.getFarPlaneDistance() * std::tan(camera.getFov() * .5f);
	float halfFarPlaneHorizontalLength = halfFarPlaneVerticalLength * camera.getAspectRatio();

	glm::vec3 topLeftFarPlane = centerOfFarPlane
		- camera.getRight() * halfFarPlaneHorizontalLength	// left offset
		+ camera.getUp() * halfFarPlaneVerticalLength;		// up offset

	glm::vec3 topRightFarPlane = centerOfFarPlane
		+ camera.getRight() * halfFarPlaneHorizontalLength	// right offset
		+ camera.getUp() * halfFarPlaneVerticalLength;		// up offset

	glm::vec3 bottomLeftFarPlane = centerOfFarPlane
		- camera.getRight() * halfFarPlaneHorizontalLength	// left offset
		- camera.getUp() * halfFarPlaneVerticalLength;		// down offset

	glm::vec3 bottomRightFarPlane = centerOfFarPlane
		+ camera.getRight() * halfFarPlaneHorizontalLength	// right offset
		- camera.getUp() * halfFarPlaneVerticalLength;		// down offset

	// -------------------------------------------------------
	// 2. Triangles visualising the frustum. (CCW)
	 
	// (Near plane) Top left triagle
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(topRightNearPlane);

	// (Near plane) Bottom right triangle
	frustumDebugPoints.push_back(topRightNearPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);

	// (Far plane) Top left triagle
	frustumDebugPoints.push_back(topLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(topRightFarPlane);

	// (Far plane) Bottom right triangle
	frustumDebugPoints.push_back(topRightFarPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Left plane) Top left triangle
	frustumDebugPoints.push_back(topLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);

	// (Left plane) Bottom right triangle
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);

	// (Right plane) Top left triangle
	frustumDebugPoints.push_back(topRightNearPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Right plane) Bottom right triangle
	frustumDebugPoints.push_back(topRightFarPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Up plane) Top left triangle
	frustumDebugPoints.push_back(topLeftFarPlane);
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(topRightFarPlane);

	// (Up plane) Bottom right triangle
	frustumDebugPoints.push_back(topRightFarPlane);
	frustumDebugPoints.push_back(topLeftNearPlane);
	frustumDebugPoints.push_back(topRightNearPlane);

	// (Bottom plane) Top left triangle
	frustumDebugPoints.push_back(bottomLeftFarPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(bottomRightFarPlane);

	// (Bottom plane) Bottom right triangle
	frustumDebugPoints.push_back(bottomRightFarPlane);
	frustumDebugPoints.push_back(bottomLeftNearPlane);
	frustumDebugPoints.push_back(bottomRightNearPlane);

	return frustumDebugPoints;
}

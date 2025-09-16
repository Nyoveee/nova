#pragma once
#undef min
#undef max
#include "export.h"
#include "camera.h"
#include "Component/component.h"
#include "type_alias.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

enum cullResult {
	isInside = -1,
	isIntersecting = 0,
	isOutside = 1
};


struct aabb {
public:
	glm::vec3 min;
	glm::vec3 max;
public:
	aabb() = default;

	aabb(glm::vec3 const& min, glm::vec3 const& max);
};

struct sphere {
public:
	glm::vec3 center;
	float radius;
public:
	sphere() = default;

	sphere(glm::vec3 const& center, float radius);
	static sphere makeBoundingSphere(const Transform& transform, float maxDimension) {
		glm::vec3 centerWS = transform.position;
		float scale = glm::max(transform.scale.x, glm::max(transform.scale.y, transform.scale.z));
		float radiusWS =  maxDimension * scale;
		return sphere(centerWS, radiusWS);
	}
};

struct plane {
public:
	glm::vec3 normal = { 0.f,1.f,0.f };
	float distance = 0.f;
public:
	plane() = default;
	plane(glm::vec3 const& point, glm::vec3 const& normal);
	plane(glm::vec3 const& normal, float dist);

	cullResult checkFrustumCulling(const sphere& sphere) const;
private:

};

struct frustum {
public:
	std::array<glm::vec3, 6>  normals;
	std::array<float, 6> dists{};
public:
	plane top;
	plane btm;
	plane right;
	plane left;
	plane farPlane;
	plane nearPlane;
public:
	frustum() = default;

	void createFrustumFromCamera(const Camera& cam, float aspect, float fovY, float zNear, float zFar);
	//static std::array<glm::vec3, 8> getFrustumCorners(const Camera& cam);

	cullResult checkFrustumCulling(const sphere& sphere) const;
	cullResult checkFrustumCulling(const aabb& aabb) const;
private:

};

struct volume {
	virtual cullResult isOnFrustum(const frustum& camFrustum, const Transform& modelTransform) const = 0;
};
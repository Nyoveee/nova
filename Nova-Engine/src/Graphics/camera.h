#pragma once

#include "export.h"
#include "type_alias.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Renderer owns Camera. The distinct camera used for rendering. Not to be confused with Camera components.
class Camera {
public:
	constexpr static glm::vec3 Up		{ 0, 1.f, 0 };

public:
	Camera();

public:
	// gets the view matrix
	ENGINE_DLL_API glm::mat4x4 const& view() const;
	ENGINE_DLL_API glm::mat4x4 const& projection() const;
	ENGINE_DLL_API glm::mat4x4 const& viewProjection() const;

	ENGINE_DLL_API glm::vec3 clipToWorldSpace(glm::vec3 const& clipPos);

public:
	// position related
	glm::vec3 const&	getPos() const;
	void				setPos(glm::vec3 pos);
	void				addPos(glm::vec3 pos);

	// camera directional vector
	glm::vec3 const&	getFront() const;
	void				setFront(glm::vec3 front);

	glm::vec3 const&	getRight() const;
	glm::vec3 const&	getUp() const;

	void				setFov(Radian angle);
	Radian				getFov() const;

	void				setNearPlaneDistance(float plane);
	float				getNearPlaneDistance() const;

	void				setFarPlaneDistance(float plane);
	float				getFarPlaneDistance() const;

	void				setAspectRatio(float aspectRatio);
	float				getAspectRatio() const;

	void				setViewMatrix(glm::mat4x4 const& viewMatrix);

	// record the current view projection matrix as previous.. (used for TAA)
	void				recordViewProjectionMatrix();
	glm::mat4 const&	getPreviousViewProjectionMatrix() const;

public:
	// only calculate view and projection matrix at the end of game loop once, for optimisation.
	void recalculateViewMatrix();
	void recalculateProjectionMatrix();

private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraRight;
	glm::vec3 cameraUp;

	glm::mat4x4 viewMatrix;

	float aspectRatio;
	float farPlaneDistance;
	float nearPlaneDistance;
	Radian fovAngle;

	glm::mat4x4 projectionMatrix;

	glm::mat4x4 viewProjectionMatrix;
	glm::mat4x4 previousViewProjectionMatrix;
};
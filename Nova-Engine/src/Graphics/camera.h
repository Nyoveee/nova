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
	ENGINE_DLL_API glm::mat4x4 view() const;
	ENGINE_DLL_API glm::mat4x4 projection() const;

public:
	// position related
	glm::vec3	getPos() const;
	void		setPos(glm::vec3 pos);
	void		addPos(glm::vec3 pos);

	// camera directional vector
	glm::vec3	getFront() const;
	void		setFront(glm::vec3 front);

	glm::vec3	getRight() const;

public:
	// only calculate view and projection matrix at the end of game loop once, for optimisation.
	void recalculateViewMatrix();
	void recalculateProjectionMatrix();

private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraRight;

	glm::mat4x4 viewMatrix;

	float aspectRatio;
	float farPlaneDistance;
	float nearPlaneDistance;
	Radian fovAngle;

	glm::mat4x4 projectionMatrix;
};
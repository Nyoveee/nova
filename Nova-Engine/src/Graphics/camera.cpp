#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "Engine/window.h"

constexpr glm::vec3 defaultCameraFront		= { 0.f, 0.f, -1.f };
constexpr Radian defaultFovAngle			= Degree{ 45.0f };
constexpr float defaultNearPlaneDistance	= 0.1f;
constexpr float defaultFarPlaneDistance		= 1000.f;
constexpr float defaultAspectRatio			= 1920.f / 1080.f;

Camera::Camera() : 
	cameraPos			{ 0.f, 1.f, 5.f },
	cameraFront			{ defaultCameraFront },
	cameraRight			{ glm::normalize(glm::cross(cameraFront, Up)) },
	viewMatrix			{},
	fovAngle			{ defaultFovAngle },
	nearPlaneDistance	{ defaultNearPlaneDistance },
	farPlaneDistance	{ defaultFarPlaneDistance },
	aspectRatio			{ defaultAspectRatio }
{
	recalculateViewMatrix();
	recalculateProjectionMatrix();
}

glm::mat4x4 Camera::view() const {
	return viewMatrix;
}

glm::mat4x4 Camera::projection() const {
	return projectionMatrix;
}

glm::vec3 Camera::getPos() const {
	return cameraPos;
}

void Camera::setPos(glm::vec3 pos) {
	cameraPos = pos;
}

void Camera::addPos(glm::vec3 pos) {
	cameraPos = cameraPos + pos;
}

glm::vec3 Camera::getFront() const {
	return cameraFront;
}

void Camera::setFront(glm::vec3 front) {
	cameraFront = front;
	cameraRight = glm::normalize(glm::cross(cameraFront, Up));
}

glm::vec3 Camera::getRight() const {
	return cameraRight;
}

float Camera::getAspectRatio() const
{
	return aspectRatio;
}

float Camera::getFov() const
{
	return fovAngle;
}

float Camera::getNearPlane() const
{
	return nearPlaneDistance;
}

float Camera::getFarPlane() const
{
	return farPlaneDistance;
}

void Camera::recalculateViewMatrix() {
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, Up);
}

void Camera::recalculateProjectionMatrix() {
	projectionMatrix = glm::perspective<float>(fovAngle, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

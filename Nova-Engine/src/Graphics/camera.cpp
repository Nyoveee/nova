#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "Engine/window.h"

constexpr glm::vec3 defaultCameraFront		= { 0.f, 0.f, -1.f };
constexpr Radian defaultFovAngle			= Degree{ 45.0f };
constexpr float defaultNearPlaneDistance	= 0.2f;
constexpr float defaultFarPlaneDistance		= 10000.f;
constexpr float defaultAspectRatio			= 1920.f / 1080.f;

Camera::Camera() : 
	cameraPos			{},
	cameraFront			{ defaultCameraFront },
	cameraRight			{ glm::normalize(glm::cross(cameraFront, Up)) },
	viewMatrix			{},
	fovAngle			{ defaultFovAngle },
	nearPlaneDistance	{ defaultNearPlaneDistance },
	farPlaneDistance	{ defaultFarPlaneDistance },
	aspectRatio			{ defaultAspectRatio }
	//isActive			{ true }
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

glm::vec3 Camera::clipToWorldSpace(glm::vec3 const& clipPos) {
	glm::vec4 clipPosVec4 = { clipPos, 1.f };
	glm::vec4 worldPos = glm::inverse(projectionMatrix * viewMatrix) * clipPosVec4;

	return glm::vec3(worldPos) / worldPos.w;
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
	cameraUp = glm::normalize(glm::cross(cameraFront, cameraRight));
}

glm::vec3 Camera::getRight() const {
	return cameraRight;
}

glm::vec3 Camera::getUp() const {
	return cameraUp;
}

void Camera::setFov(Radian angle) {
	fovAngle = angle;
}

Radian Camera::getFov() const {
	return fovAngle;
}

void Camera::setNearPlaneDistance(float plane) {
	nearPlaneDistance = plane;
}

float Camera::getNearPlaneDistance() const {
	return nearPlaneDistance;
}

void Camera::setFarPlaneDistance(float plane) {
	farPlaneDistance = plane;
}

float Camera::getFarPlaneDistance() const {
	return farPlaneDistance;
}

float Camera::getAspectRatio() const {
	return aspectRatio;
}

void Camera::recalculateViewMatrix() {
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, Up);
}

void Camera::recalculateProjectionMatrix() {
	projectionMatrix = glm::perspective<float>(fovAngle, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

//bool Camera::getStatus() {
//	return isActive;
//}
//
//void Camera::setStatus(bool newStatus) {
//	isActive = newStatus;
//}

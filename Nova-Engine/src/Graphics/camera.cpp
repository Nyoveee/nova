#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "Engine/window.h"

constexpr glm::vec3 defaultCameraFront		= { 0.f, 0.f, -1.f };
constexpr Radian defaultFovAngle			= Degree{ 45.0f };
constexpr float defaultNearPlaneDistance	= 0.2f;
constexpr float defaultFarPlaneDistance		= 3000.f;
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

glm::mat4x4 const& Camera::view() const {
	return viewMatrix;
}

glm::mat4x4 const& Camera::projection() const {
	return projectionMatrix;
}

glm::mat4x4 const& Camera::viewProjection() const {
	return viewProjectionMatrix;
}

glm::vec3 Camera::clipToWorldSpace(glm::vec3 const& clipPos) {
	glm::vec4 clipPosVec4 = { clipPos, 1.f };
	glm::vec4 worldPos = glm::inverse(projectionMatrix * viewMatrix) * clipPosVec4;

	return glm::vec3(worldPos) / worldPos.w;
}

glm::vec3 const& Camera::getPos() const {
	return cameraPos;
}

void Camera::setPos(glm::vec3 pos) {
	cameraPos = pos;
}

void Camera::addPos(glm::vec3 pos) {
	cameraPos = cameraPos + pos;
}

glm::vec3 const& Camera::getFront() const {
	return cameraFront;
}

void Camera::setFront(glm::vec3 front) {
	cameraFront = front;
	cameraRight = glm::normalize(glm::cross(cameraFront, Up));
	cameraUp = glm::normalize(glm::cross(cameraFront, cameraRight));
}

glm::vec3 const& Camera::getRight() const {
	return cameraRight;
}

glm::vec3 const& Camera::getUp() const {
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

void Camera::setAspectRatio(float p_aspectRatio) {
	aspectRatio = p_aspectRatio;
}

float Camera::getAspectRatio() const {
	return aspectRatio;
}

void Camera::setViewMatrix(glm::mat4x4 const& p_viewMatrix) {
	viewMatrix = p_viewMatrix;
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Camera::recordViewProjectionMatrix() {
	previousViewProjectionMatrix = viewProjectionMatrix;
}

glm::mat4 const& Camera::getPreviousViewProjectionMatrix() const {
	return previousViewProjectionMatrix;
}

void Camera::updateCameraShake(float deltaTime) {
	if (!cameraShake.active) {
		return;
	}

	cameraShake.currentDuration -= deltaTime;

	// powering the interval for quicker fall off.
	cameraShake.currentAmplification = std::lerp(0.f, cameraShake.amplification, std::powf(cameraShake.currentDuration / cameraShake.duration, 3.f));

	float offset = cameraShake.currentAmplification * std::cos(cameraShake.currentDuration);

	cameraShake.positionOffset = glm::vec3{
		offset * static_cast<float>(std::rand()) / RAND_MAX,
		offset * static_cast<float>(std::rand()) / RAND_MAX,
		offset * static_cast<float>(std::rand()) / RAND_MAX
	};

	if (cameraShake.currentDuration <= 0.f) {
		cameraShake.active = false;
		cameraShake.positionOffset = {};
	}
}

void Camera::setCameraShake(float duration, float amplification) {
	if (duration == 0) {
		cameraShake.active = false;
		return;
	}

	cameraShake.active = true;
	cameraShake.duration = duration;
	cameraShake.amplification = amplification;
	cameraShake.currentDuration = duration;
	cameraShake.currentAmplification = amplification;
}

void Camera::recalculateViewMatrix() {
	glm::vec3 pos = cameraPos + cameraShake.positionOffset;
	viewMatrix = glm::lookAt(pos, pos + cameraFront, Up);
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Camera::recalculateProjectionMatrix() {
	projectionMatrix = glm::perspective<float>(fovAngle, aspectRatio, nearPlaneDistance, farPlaneDistance);
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}
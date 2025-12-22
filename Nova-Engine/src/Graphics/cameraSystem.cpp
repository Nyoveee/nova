#include <cmath>
#include <glm/gtc/matrix_access.hpp>

#include "Engine/engine.h"
#include "cameraSystem.h"
#include "renderer.h"
#include "InputManager/inputManager.h"
#include "Profiling.h"
#include "frustum.h"

namespace {
	constexpr float sensitivity = 0.1f;		// change this value to your liking

	glm::vec3 findMaxBound(glm::vec3 boundOne) {
		return boundOne;
	}

	template <typename ...Args>
	glm::vec3 findMaxBound(glm::vec3 bound, Args... bounds) {
		return glm::max(bound, findMaxBound(bounds...));
	}

	glm::vec3 findMinBound(glm::vec3 boundOne) {
		return boundOne;
	}

	template <typename ...Args>
	glm::vec3 findMinBound(glm::vec3 bound, Args... bounds) {
		return glm::min(bound, findMinBound(bounds...));
	}
}

CameraSystem::CameraSystem(Engine& engine) :
	engine					{ engine },
	isMovingDown			{},
	isMovingUp				{},
	isMovingFront			{},
	isMovingBack			{},
	isMovingLeft			{},
	isMovingRight			{},
	isSimulationActive		{},
	editorCamera			{ engine.renderer.getEditorCamera() },
	gameCamera				{ engine.renderer.getGameCamera() },
	levelEditorCamera		{ { 0.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, -90.f, 0.f },
	isInControl				{ false },
	isThereActiveGameCamera	{ false },
	toResetMousePos			{ true },
	cameraSpeedExponent		{ 2.f },
	cameraSpeed				{ std::exp(cameraSpeedExponent) },
	focusOffsetDistance		{ 5.0f },
	focusHeightOffset		{ 2.0f }
{
	// Subscribe to the input manager that the camera system is interested in 
	// any input related to CameraMovement
	engine.inputManager.subscribe<CameraMovement>(
		[&](CameraMovement movement) {
			setMovement(movement, true);
		},
		[&](CameraMovement movement) {
			setMovement(movement, false);
		}
	);

	engine.inputManager.subscribe<ToCameraControl>(
		[&](ToCameraControl control) {
			if (isSimulationActive && isThereActiveGameCamera) {
				return;
			}

			isInControl = control == ToCameraControl::Control;

			if (isInControl) {
				toResetMousePos = true;
			}
		}
	);

	engine.inputManager.subscribe<MousePosition>(
		[&](MousePosition mousePos) {
			if (isInControl) {
				calculateEulerAngle(static_cast<float>(mousePos.xPos), static_cast<float>(mousePos.yPos));
			}
		}
	);

	engine.inputManager.subscribe<AdjustCameraSpeed>(
		[&](AdjustCameraSpeed value) {
			if (!isInControl) {
				return;
			}

			constexpr float adjustmentMultiplier = 0.1f;
			cameraSpeedExponent += static_cast<float>(value.value) * adjustmentMultiplier;
		}
	);
}

void CameraSystem::update(float dt) {
#if defined(DEBUG)
	ZoneScoped;
#endif

	for (auto&& [entityID, transform, cameraComponent] : engine.ecs.registry.view<Transform, CameraComponent>().each()) {
		if (!cameraComponent.camStatus) {
			continue;
		}
		
		gameCamera.setPos(transform.position);
		gameCamera.setFront(transform.front);
		gameCamera.setFov(cameraComponent.fov);
		gameCamera.setNearPlaneDistance(cameraComponent.nearPlane);
		gameCamera.setFarPlaneDistance(cameraComponent.farPlane);

		gameCamera.recalculateViewMatrix();
		gameCamera.recalculateProjectionMatrix();
		
		// We perform frustum culling..
		frustumCulling(calculateGameCameraFrustum());

		break;
	}

	// for editor camera
	if(!isSimulationActive || !isThereActiveGameCamera)
	{
		if (!isInControl) {
			return;
		}

		cameraSpeed = std::exp(cameraSpeedExponent);

		if (isMovingFront) {
			levelEditorCamera.position += cameraSpeed * dt * editorCamera.getFront();
		}

		if (isMovingLeft) {
			levelEditorCamera.position -= cameraSpeed * dt * editorCamera.getRight();
		}

		if (isMovingBack) {
			levelEditorCamera.position -= cameraSpeed * dt * editorCamera.getFront();
		}

		if (isMovingRight) {
			levelEditorCamera.position += cameraSpeed * dt * editorCamera.getRight();
		}

		if (isMovingUp) {
			levelEditorCamera.position += cameraSpeed * dt * Camera::Up;
		}

		if (isMovingDown) {
			levelEditorCamera.position -= cameraSpeed * dt * Camera::Up;
		}
	
		editorCamera.setPos(levelEditorCamera.position);
		editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	}
	
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();

}

void CameraSystem::setMovement(CameraMovement movement, bool toMove) {
	switch (movement)
	{
	case CameraMovement::Forward:
		isMovingFront = toMove;
		break;
	case CameraMovement::Backward:
		isMovingBack = toMove;
		break;
	case CameraMovement::Left:
		isMovingLeft = toMove;
		break;
	case CameraMovement::Right:
		isMovingRight = toMove;
		break;
	case CameraMovement::Ascend:
		isMovingUp = toMove;
		break;
	case CameraMovement::Descent:
		isMovingDown = toMove;
		break;
	default:
		break;
	}
}

float CameraSystem::getCameraSpeed() const {
	return cameraSpeed;
}

CameraSystem::LevelEditorCamera const& CameraSystem::getLevelEditorCamera() const {
    return levelEditorCamera;
}

void CameraSystem::frustumCulling(Frustum gameCameraFrustum) {
	auto calculateFrustumCulling = [&](Model* model, Transform& transform) {
		if (!model) {
			return;
		}

		// Calculate appropriate bounding box.
		transform.boundingBox = calculateAABB(*model, transform);
		transform.inCameraFrustum = gameCameraFrustum.isAABBInFrustum(transform.boundingBox);
	};

	for (auto&& [entityID, entityData, transform, meshRenderer] : engine.ecs.registry.view<EntityData, Transform, MeshRenderer>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entityID)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = engine.resourceManager.getResource<Model>(meshRenderer.modelId);
		calculateFrustumCulling(model, transform);
	}

	for (auto&& [entityID, entityData, transform, skinnedMeshRenderer] : engine.ecs.registry.view<EntityData, Transform, SkinnedMeshRenderer>().each()) {
		// pointless to do frustum culling on disabled objects.
		if (!entityData.isActive || !engine.ecs.isComponentActive<MeshRenderer>(entityID)) {
			continue;
		}

		// Retrieves model asset from asset manager.
		auto [model, _] = engine.resourceManager.getResource<Model>(skinnedMeshRenderer.modelId);
		calculateFrustumCulling(model, transform);
	}
}

Frustum CameraSystem::calculateGameCameraFrustum() {
	Frustum frustum;

	// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	// https://www.reddit.com/r/opengl/comments/1fstgtt/strange_issue_with_frustum_extraction/
	glm::mat4x4 m = gameCamera.projection() * gameCamera.view();

	frustum.leftPlane	= { glm::row(m, 3) + glm::row(m, 0) };
	frustum.rightPlane	= { glm::row(m, 3) - glm::row(m, 0) };
	frustum.bottomPlane = { glm::row(m, 3) + glm::row(m, 1) };
	frustum.topPlane	= { glm::row(m, 3) - glm::row(m, 1) };
	frustum.nearPlane	= { glm::row(m, 3) + glm::row(m, 2) };
	frustum.farPlane	= { glm::row(m, 3) - glm::row(m, 2) };

	frustum.leftPlane.normalize();
	frustum.rightPlane.normalize();
	frustum.bottomPlane.normalize();
	frustum.topPlane.normalize();
	frustum.nearPlane.normalize();
	frustum.farPlane.normalize();
	
	return frustum;
}

AABB CameraSystem::calculateAABB(Model const& model, Transform const& transform) {
	if (transform.rotation == glm::quat_identity<float, glm::highp>()) {
		return {
			model.center * transform.scale + transform.position,
			model.extents * transform.scale
		};
	}

	// https://stackoverflow.com/questions/34619341/can-axis-aligned-bounding-boxes-be-recalculated-after-rotation-of-object-using-t
	// We need to transform our AABBs
	glm::vec3 pointOne		= model.maxBound;
	glm::vec3 pointTwo		= glm::vec3{ model.maxBound.x, model.maxBound.y, model.minBound.z };
	glm::vec3 pointThree	= glm::vec3{ model.maxBound.x, model.minBound.y, model.minBound.z };
	glm::vec3 pointFour		= glm::vec3{ model.maxBound.x, model.minBound.y, model.maxBound.z };
	glm::vec3 pointFive		= glm::vec3{ model.minBound.x, model.minBound.y, model.maxBound.z };
	glm::vec3 pointSix		= glm::vec3{ model.minBound.x, model.maxBound.y, model.maxBound.z };
	glm::vec3 pointSeven	= glm::vec3{ model.minBound.x, model.maxBound.y, model.minBound.z };
	glm::vec3 pointEight	= model.minBound;

	pointOne	= transform.rotation * (transform.scale * pointOne);
	pointTwo	= transform.rotation * (transform.scale * pointTwo);
	pointThree	= transform.rotation * (transform.scale * pointThree);
	pointFour	= transform.rotation * (transform.scale * pointFour);
	pointFive	= transform.rotation * (transform.scale * pointFive);
	pointSix	= transform.rotation * (transform.scale * pointSix);
	pointSeven	= transform.rotation * (transform.scale * pointSeven);
	pointEight	= transform.rotation * (transform.scale * pointEight);

	glm::vec3 newMaxBound = findMaxBound(pointOne, pointTwo, pointThree, pointFour, pointFive, pointSix, pointSeven, pointEight);
	glm::vec3 newMinBound = findMinBound(pointOne, pointTwo, pointThree, pointFour, pointFive, pointSix, pointSeven, pointEight);
	glm::vec3 newCenter = (newMaxBound + newMinBound) / 2.f;
	glm::vec3 newExtent = newMaxBound - newCenter;

	return {
		newCenter + transform.position,
		newExtent
	};

	//glm::vec3 rotatedCenter = transform.rotation * (transform.scale * model->center);

	//glm::vec3 extents = [&]() {
	//	if (transform.rotation == glm::quat_identity<float, glm::highp>()) {
	//		return transform.scale * model->extents;
	//	}

	//	glm::vec3 rotatedMax = transform.rotation * (transform.scale * model->maxBound);
	//	return rotatedMax - rotatedCenter;
	//}();
}

void CameraSystem::calculateEulerAngle(float xOffset, float yOffset) {
	levelEditorCamera.yaw += xOffset * sensitivity;
	levelEditorCamera.pitch += yOffset * sensitivity;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	levelEditorCamera.pitch = std::clamp(static_cast<float>(levelEditorCamera.pitch), -89.0f, 89.0f);

	levelEditorCamera.front.x = cos(Radian{ levelEditorCamera.yaw }) * cos(Radian{ levelEditorCamera.pitch });
	levelEditorCamera.front.y = sin(Radian{ levelEditorCamera.pitch });
	levelEditorCamera.front.z = sin(Radian{ levelEditorCamera.yaw }) * cos(Radian{ levelEditorCamera.pitch });
}


void CameraSystem::startSimulation() {
	isSimulationActive = true;
}

void CameraSystem::endSimulation() {
	isSimulationActive = false;

	editorCamera.setPos(levelEditorCamera.position);
	editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();
}

void CameraSystem::focusOnPosition(glm::vec3 const& targetPosition) {
	// Position camera behind and above the target using configurable offsets
	glm::vec3 offset = glm::vec3(0.0f, focusHeightOffset, focusOffsetDistance);
	levelEditorCamera.position = targetPosition + offset;

	// Calculate direction from camera to target
	glm::vec3 direction = glm::normalize(targetPosition - levelEditorCamera.position);
	levelEditorCamera.front = direction;

	// Calculate yaw and pitch from direction vector
	// yaw: angle around y-axis (horizontal rotation)
	levelEditorCamera.yaw = Degree{ glm::degrees(std::atan2(direction.z, direction.x)) };

	// pitch: angle up/down
	levelEditorCamera.pitch = Degree{ glm::degrees(std::asin(direction.y)) };

	// Update the editor camera immediately
	editorCamera.setPos(levelEditorCamera.position);
	editorCamera.setFront(glm::normalize(levelEditorCamera.front));
	editorCamera.recalculateViewMatrix();
	editorCamera.recalculateProjectionMatrix();
}
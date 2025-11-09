#include "ScriptLibrary.hxx"

// ======================================
// Time..
// ======================================
float Time::V_FixedDeltaTime() { return 1 / 60.f; } // Replace with config
float Time::V_DeltaTime() { return Interface::engine->getDeltaTime(); } // Replace with config

// ======================================
// Debug and logging..
// ======================================
void Debug::Log(IManagedComponent^ component) { Logger::info(Convert(component->ToString())); }
void Debug::Log(System::String^ string) { Logger::info(Convert(string)); }
void Debug::Log(Object^ object) {
	if (object->GetType()->IsPrimitive) {
		Logger::info(Convert(object->ToString()));
		return;
	}
	Logger::info(Convert(object->GetType()->Name) + Convert(object->ToString()));
}

void Debug::LogWarning(IManagedComponent^ component) { Logger::warn(Convert(component->ToString())); }
void Debug::LogWarning(System::String^ string) { Logger::warn(Convert(string)); }
void Debug::LogWarning(Object^ object) {
	if (object->GetType()->IsPrimitive) {
		Logger::warn(Convert(object->ToString()));
		return;
	}
	Logger::warn(Convert(object->GetType()->Name) + Convert(object->ToString()));
}

void Debug::LogError(IManagedComponent^ component) { Logger::warn(Convert(component->ToString())); }
void Debug::LogError(System::String^ string) { Logger::warn(Convert(string)); }
void Debug::LogError(Object^ object) {
	if (object->GetType()->IsPrimitive) {
		Logger::warn(Convert(object->ToString()));
		return;
	}
	Logger::warn(Convert(object->GetType()->Name) + Convert(object->ToString()));
}

// ======================================
// Input management..
// ======================================

std::size_t Input::MapKey(Key key, EventCallback^ pressCallback) {
	std::size_t observerId{ Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key)) };
	scriptObserverIds.Add(observerId);
	return observerId;
}

std::size_t Input::MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback) {
	std::size_t observerId{ Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key), Convert<ScriptingInputEvents>(releaseCallback, key)) };
	scriptObserverIds.Add(observerId);
	return observerId;
}

std::size_t Input::MouseMoveCallback(MouseEventCallback^ callback) {
	std::size_t observerId{ Interface::engine->inputManager.subscribe<MousePosition>(CreateMouseCallback(callback)) };
	mouseMoveObserverIds.Add(observerId);
	return observerId;
}

std::size_t Input::ScrollCallback(ScrollEventCallback^ callback) {
	std::size_t observerId{ Interface::engine->inputManager.subscribe<Scroll>(CreateScrollCallback(callback)) };
	mouseScrollObserverIds.Add(observerId);
	return observerId;
}

Vector2 Input::V_MousePosition() { return Vector2(Interface::engine->inputManager.mousePosition); }

void Input::ClearAllKeyMapping() {
	for each (std::size_t observerId in scriptObserverIds) {
		Interface::engine->inputManager.unsubscribe<ScriptingInputEvents>(ObserverID{ observerId });
	}

	for each (std::size_t observerId in mouseMoveObserverIds) {
		Interface::engine->inputManager.unsubscribe<MousePosition>(ObserverID{ observerId });
	}

	for each (std::size_t observerId in mouseScrollObserverIds) {
		Interface::engine->inputManager.unsubscribe<Scroll>(ObserverID{ observerId });
	}

	scriptObserverIds.Clear();
	mouseMoveObserverIds.Clear();
	mouseScrollObserverIds.Clear();
}

// ======================================
// Audio APIs..
// ======================================
void AudioAPI::PlaySound(GameObject^ gameObject, System::String^ string) {
	Logger::info("{}", static_cast<unsigned>(gameObject->entityID));
	Interface::engine->audioSystem.playSFX(Convert(gameObject), Convert(string));
}
void AudioAPI::PlayBGM(GameObject^ gameObject, System::String^ string) {
	Interface::engine->audioSystem.playBGM(Convert(gameObject), Convert(string));
}
void AudioAPI::StopSound(GameObject^ gameObject, System::String^ string) {
	Interface::engine->audioSystem.stopSound(Convert(gameObject), Convert(string));
}

// ======================================
// Physics APIs..
// ======================================
System::Nullable<RayCastResult> PhysicsAPI::Raycast(Vector3 origin, Vector3 directionVector, float maxDistance) {
	return Raycast(Ray{ origin, directionVector }, maxDistance, {});
}

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, GameObject^ entityToIgnore) {
	return Raycast(Ray{ origin, directionVector }, maxDistance, entityToIgnore);
}

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Ray^ p_ray, float maxDistance) {
	PhysicsRay ray{ p_ray->native() };
	auto opt = Interface::engine->physicsManager.rayCast(ray, maxDistance);

	if (!opt) {
		return {}; // returns null, no ray cast..
	}

	return System::Nullable<RayCastResult>(RayCastResult{ opt.value() });
}

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Ray^ p_ray, float maxDistance, GameObject^ entityToIgnore) {
	PhysicsRay ray{ p_ray->native() };
	std::optional<PhysicsRayCastResult> opt;

	if (entityToIgnore) {
		opt = Interface::engine->physicsManager.rayCast(ray, maxDistance, { static_cast<entt::entity>(entityToIgnore->entityID) });
	}
	else {
		opt = Interface::engine->physicsManager.rayCast(ray, maxDistance, {} );
	}

	if (!opt) {
		return {}; // returns null, no ray cast..
	}

	return System::Nullable<RayCastResult>(RayCastResult{ opt.value() });
}

// ======================================
// Camera related APIs
// ======================================

Ray CameraAPI::getRayFromMouse() {
	auto ray = Interface::engine->physicsManager.getRayFromMouse();
	return Ray{ ray };
}

void CameraAPI::LockMouse() {
	Interface::engine->gameLockMouse(true);
}

void CameraAPI::UnlockMouse() {
	Interface::engine->gameLockMouse(false);
}

// ======================================
// This class is responsible for providing Navigation related APIs to the script.
// ======================================
bool NavigationAPI::setDestination(GameObject^ gameObject, Vector3^ targetPosition) {
	return Interface::engine->navigationSystem.setDestination(Convert(gameObject), targetPosition->native());
}

// ======================================
// Math related API.. hehe? :D
// ======================================
float Mathf::Cos(float radian) { return std::cos(radian); }
float Mathf::Sin(float radian) { return std::sin(radian); }
float Mathf::Atan2(float y, float x) { return std::tan(y / x); }
float Mathf::Clamp(float value, float min, float max) { return std::clamp(value, min, max); }
float Mathf::Interpolate(float a, float b, float t, float degree) { return Interpolation::Interpolation(a, b, t, degree); }
float Mathf::Min(float a, float b) { return std::min(a, b); }
float Mathf::Max(float a, float b) { return std::max(a, b); }
float Mathf::Pow(float base, float exponent) { return std::powf(base, exponent); }

// ======================================
// Game Object creation and deletion..
// ======================================

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab) {
	return Instantiate(prefab, nullptr);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent) {
	constexpr glm::vec3 zeroPos = { 0.f, 0.f, 0.f };
	return Instantiate(prefab, gcnew Vector3{ zeroPos }, parent);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent) {
	constexpr glm::quat identityQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
	return Instantiate(prefab, localPosition, gcnew Quartenion{ identityQuat }, parent);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quartenion^ localRotation, GameObject^ parent) {
	entt::entity prefabInstanceId = Interface::engine->prefabManager.instantiatePrefab<ALL_COMPONENTS>(prefab->getId());

	// set parent, and local transform..
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(prefabInstanceId);
	Transform* transform = Interface::engine->ecs.registry.try_get<Transform>(prefabInstanceId);

	if (!entityData || !transform) {
		Logger::warn("Failed to instantiate entity?");
		return nullptr;
	}

	if (parent && static_cast<entt::entity>(parent->entityID) != entt::null) {
		Interface::engine->ecs.setEntityParent(prefabInstanceId, static_cast<entt::entity>(parent->entityID));
		transform->localPosition = localPosition->native();
		transform->localRotation = localRotation->native();
	}
	else {
		transform->position = localPosition->native();
		transform->rotation = localRotation->native();
	}

	// initialise all scripts..
	Scripts* scripts = Interface::engine->ecs.registry.try_get<Scripts>(prefabInstanceId);

	if (scripts) {
		// Instantiate these scripts and call init..
		for (auto&& scriptData : scripts->scriptDatas) {
			Interface::addEntityScript(static_cast<unsigned>(prefabInstanceId), static_cast<unsigned long long>(scriptData.scriptId));

			for (auto&& fieldData : scriptData.fields)
				Interface::setScriptFieldData(static_cast<unsigned>(prefabInstanceId), static_cast<unsigned long long>(scriptData.scriptId), fieldData);

			Interface::initializeScript(static_cast<unsigned>(prefabInstanceId), static_cast<unsigned long long>(scriptData.scriptId));
		}
	}

	// yoinked from zhi wei
	GameObject^ newGameObject = gcnew GameObject(prefabInstanceId);

	return newGameObject;
}

void ObjectAPI::Destroy(GameObject^ gameObject) {
	Interface::submitGameObjectDeleteRequest(gameObject->entityID);
}

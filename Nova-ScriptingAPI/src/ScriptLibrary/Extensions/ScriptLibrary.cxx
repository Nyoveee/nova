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
	if (!object) {
		Logger::error("Unable to Print, Object is null");
		return;
	}
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

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, array<System::String^>^ layermask)
{
	std::vector<uint8_t> layerIds;

	for each (System::String ^ layer in layermask) {
		std::string layerName = Convert(layer);
		layerIds.push_back(static_cast<uint8_t>(magic_enum::enum_cast<Rigidbody::Layer>(layerName.c_str()).value()));
	}
	auto opt = Interface::engine->physicsManager.rayCast(Ray{origin,directionVector}.native(), maxDistance, layerIds);
	if (!opt)
		return {};
	return System::Nullable<RayCastResult>(RayCastResult{ opt.value() });
}

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, GameObject^ entityToIgnore) {
	return Raycast(Ray{ origin, directionVector }, maxDistance, entityToIgnore);
}

System::Nullable<RayCastResult> PhysicsAPI::Raycast(Ray^ p_ray, float maxDistance) {
	PhysicsRay ray{ p_ray->native() };
	auto opt = Interface::engine->physicsManager.rayCast(ray, maxDistance, std::vector<entt::entity>{});

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
		opt = Interface::engine->physicsManager.rayCast(ray, maxDistance, std::vector<entt::entity>{});
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
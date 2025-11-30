#include "ScriptLibrary.hxx"
#include "nova_math.h"

// ======================================
// Time..
// ======================================
float Time::V_FixedDeltaTime()			{ return (1 / 60.f) * Interface::engine->deltaTimeMultiplier; } // Replace with config
float Time::V_DeltaTime()				{ return Interface::engine->getDeltaTime() * Interface::engine->deltaTimeMultiplier; }
float Time::V_AccumulatedTime()			{ return Interface::engine->getAccumulatedTime() * Interface::engine->deltaTimeMultiplier; }

float Time::V_AccumulatedTime_Unscaled()			{ return Interface::engine->getAccumulatedTime(); }
float Time::V_FixedDeltaTime_Unscaled() { return 1 / 60.f; }
float Time::V_DeltaTime_Unscaled()		{ return Interface::engine->getDeltaTime(); }

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

Vector2 Input::GetUIMousePosition() {
	return Vector2{ Interface::engine->getUIMousePosition() };
}

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

System::Nullable<RayCastResult> PhysicsAPI::Linecast(Vector3 start, Vector3 end)
{
	return Linecast(start, end, {});
}

System::Nullable<RayCastResult> PhysicsAPI::Linecast(Vector3 start, Vector3 end, array<System::String^>^ layermask)
{
	float distance = Vector3::Distance(start, end);
	Vector3 direction = Vector3::operator-(end, start);
	direction.Normalize();
	return Raycast(start,direction,distance,layermask);
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

void NavigationAPI::stopAgent(GameObject^ gameObject)
{
	Interface::engine->navigationSystem.stopAgent(Convert(gameObject));
}

// ======================================
// Math related API.. hehe? :D
// ======================================
float Mathf::Cos		(float radian)								{ return std::cos(radian); }
float Mathf::Sin		(float radian)								{ return std::sin(radian); }
float Mathf::Atan2		(float y, float x)							{ return std::tan(y / x); }
float Mathf::Clamp		(float value, float min, float max)			{ return std::clamp(value, min, max); }
float Mathf::Interpolate(float a, float b, float t, float degree)	{ return Interpolation::Interpolation(a, b, t, degree); }
float Mathf::Min		(float a, float b)							{ return std::min(a, b); }
float Mathf::Max		(float a, float b)							{ return std::max(a, b); }
float Mathf::Pow		(float base, float exponent)				{ return std::powf(base, exponent); }
float Mathf::Abs		(float value)								{ return std::abs(value); }
float Mathf::SmoothLerp	(float a, float b, float t)					{ return Interpolation::Interpolation(a, b, Math::smoothstep(t), 1); }
float Mathf::Sqrt		(float f)									{ return std::sqrt(f); }

// ======================================
// Random Related API
// ======================================
float Random::Range(float minInclusive, float maxInclusive)
{
	return RandomRange::Float(minInclusive,maxInclusive);
}

int Random::Range(int minInclusive, int maxExclusive)
{
	return RandomRange::Int(minInclusive,maxExclusive);
}

// ======================================
// Scene related API
// ======================================
void SceneAPI::ChangeScene(ScriptingAPI::Scene^ sceneId) {
	// We want to delay our change scene request.
	Interface::changeSceneRequest(sceneId);
}


// =================================================================
// Player Preference.
// =================================================================

float PlayerPrefs::GetFloat(System::String^ key)
{
	return GetFloat(key, 0.f);
}

int PlayerPrefs::GetInt(System::String^ key)
{
	return GetInt(key, 0);
}

System::String^ PlayerPrefs::GetString(System::String^ key)
{
	return GetString(key, "");
}

float PlayerPrefs::GetFloat(System::String^ key, float defaultValue)
{
	auto dataOpt = Interface::engine->dataManager.loadFloatData(Convert(key));
	return dataOpt.value_or(defaultValue);
}

int PlayerPrefs::GetInt(System::String^ key, int defaultValue)
{
	auto dataOpt = Interface::engine->dataManager.loadIntData(Convert(key));
	return dataOpt.value_or(defaultValue);
}

System::String^ PlayerPrefs::GetString(System::String^ key, System::String^ defaultValue)
{
	auto dataOpt = Interface::engine->dataManager.loadStringData(Convert(key));

	if (!dataOpt) {
		return defaultValue;
	}
	else {
		return msclr::interop::marshal_as<System::String^>(dataOpt.value());
	}
}

void PlayerPrefs::SetFloat(System::String^ key, float value)
{
	Interface::engine->dataManager.saveData(Convert(key), value);
}

void PlayerPrefs::SetInt(System::String^ key, int value)
{
	Interface::engine->dataManager.saveData(Convert(key), value);
}

void PlayerPrefs::SetString(System::String^ key, System::String^ value)
{
	Interface::engine->dataManager.saveData(Convert(key), Convert(value));
}

void PlayerPrefs::Save()
{
	Interface::engine->dataManager.savePlayerPreference();
}

void PlayerPrefs::DeleteKey(System::String^ key) {
	Interface::engine->dataManager.removeKey(Convert(key));
}

void PlayerPrefs::DeleteAll() {
	Interface::engine->dataManager.clear();
}

// =================================================================
// Systems related API.
// =================================================================
void Systems::Restart()
{
	Interface::engine->ecs.sceneManager.restartScene();
}

void Systems::Quit()
{
	Interface::engine->quit();
}

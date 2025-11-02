#pragma once

#include "Logger.h"
#include "ManagedTypes.hxx"
#include "API/IManagedComponent.hxx"
#include "API/ConversionUtils.hxx"
#include "API/ScriptingAPI.hxx"
#include "InputManager/inputManager.h"
#include "Interpolation.h"

#include <numbers>

#undef PlaySound

/*******************************************************************************************
	For this library, you would need to understand the difference between
	Managed Types(For C#) and Native Types(C++)

	To Create Managed Type Versions based on Native Versions, go to ManagedTypes.hxx

 	For Variables Indicate with V_

	Conversion from Managed to native types is provided:
		Component: Convert(Managed Component)
		GameObject: Convert(GameObject)
		Managed Struct: VariableName->native()
		Primitives: Converted by default for these types https://learn.microsoft.com/en-us/cpp/dotnet/managed-types-cpp-cli?view=msvc-170
		Strings: Convert(Managed String)
		Event Callbacks: Convert<Event>(Managed Callback,key)

	Recommended to only have one line which is to simply call the engine function
*******************************************************************************************/
public ref class Time {
public:
	static float V_FixedDeltaTime() { return 1 / 60.f; } // Replace with config
	static float V_DeltaTime()		{ return Interface::engine->getDeltaTime(); } // Replace with config
};

public ref class Debug {
public:
	static void Log(IManagedComponent^ component) { Logger::info(Convert(component->ToString()));}
	static void Log(System::String^ string)		{ Logger::info(Convert(string)); }
	static void Log(Object^ object)               { 
		if (object->GetType()->IsPrimitive) {
			Logger::info(Convert(object->ToString()));
			return;
		}
		Logger::info(Convert(object->GetType()->Name) + Convert(object->ToString())); 
	}
	static void LogWarning(IManagedComponent^ component) { Logger::warn(Convert(component->ToString())); }
	static void LogWarning(System::String^ string) { Logger::warn(Convert(string)); }
	static void LogWarning(Object^ object) {
		if (object->GetType()->IsPrimitive) {
			Logger::warn(Convert(object->ToString()));
			return;
		}
		Logger::warn(Convert(object->GetType()->Name) + Convert(object->ToString()));
	}
	static void LogError(IManagedComponent^ component) { Logger::warn(Convert(component->ToString())); }
	static void LogError(System::String^ string) { Logger::warn(Convert(string)); }
	static void LogError(Object^ object) {
		if (object->GetType()->IsPrimitive) {
			Logger::warn(Convert(object->ToString()));
			return;
		}
		Logger::warn(Convert(object->GetType()->Name) + Convert(object->ToString()));
	}
};

// ======================================
// This class is responsible for providing input related APIs to the script.
// ======================================

public ref class Input {
public:
	static void MapKey(Key key, EventCallback^ pressCallback) {
		std::size_t observerId{ Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key)) };
		scriptObserverIds.Add(observerId);
	}

	static void MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback) { 
		std::size_t observerId{ Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key), Convert<ScriptingInputEvents>(releaseCallback, key)) };
		scriptObserverIds.Add(observerId);
	}

	static void MouseMoveCallback(MouseEventCallback^ callback) {
		std::size_t observerId{ Interface::engine->inputManager.subscribe<MousePosition>(CreateMouseCallback(callback)) };
		mouseMoveObserverIds.Add(observerId);
	}

	static void ScrollCallback(ScrollEventCallback^ callback) {
		std::size_t observerId{ Interface::engine->inputManager.subscribe<Scroll>(CreateScrollCallback(callback)) };
		mouseScrollObserverIds.Add(observerId);
	}

#if 0
	static float GetMouseAxis(MouseAxis axis) {
		if (axis == MouseAxis::Horizontal) 
			return Interface::engine->cameraSystem.getMouseAxisX();
		return Interface::engine->cameraSystem.getMouseAxisY();
	}

	static float GetMouseAxisRaw(MouseAxis axis) {
		float output{ GetMouseAxis(axis) };
		if (output > 0) return 1;
		if (output < 0) return -1;
		return 0.f;
	}

	static float V_ScrollOffsetY()		{ return Interface::engine->inputManager.scrollOffsetY; }
#endif
	static Vector2 V_MousePosition()	{ return Vector2(Interface::engine->inputManager.mousePosition); }

internal:
	static void ClearAllKeyMapping() {
		for each (std::size_t observerId in scriptObserverIds) {
			Interface::engine->inputManager.unsubscribe<ScriptingInputEvents>(ObserverID{ observerId });
		}

		for each (std::size_t observerId in mouseMoveObserverIds) {
			Interface::engine->inputManager.unsubscribe<ScriptingInputEvents>(ObserverID{ observerId });
		}

		for each (std::size_t observerId in mouseScrollObserverIds) {
			Interface::engine->inputManager.unsubscribe<ScriptingInputEvents>(ObserverID{ observerId });
		}

		scriptObserverIds.Clear();
		mouseMoveObserverIds.Clear();
		mouseScrollObserverIds.Clear();
	}

private:
	static System::Collections::Generic::List<std::size_t> scriptObserverIds;
	static System::Collections::Generic::List<std::size_t> mouseMoveObserverIds;
	static System::Collections::Generic::List<std::size_t> mouseScrollObserverIds;

};

// ======================================
// This class is responsible for providing audio related APIs to the script.
// ======================================
public ref class AudioAPI {
public:
	static void PlaySound(GameObject^ gameObject, System::String^ string) {
		Interface::engine->audioSystem.playSFX(Convert(gameObject), Convert(string));
	}
	static void PlayBGM(GameObject^ gameObject, System::String^ string) {
		Interface::engine->audioSystem.playBGM(Convert(gameObject), Convert(string));
	}
	static void StopSound(GameObject^ gameObject, System::String^ string) {
		Interface::engine->audioSystem.stopSound(Convert(gameObject), Convert(string));
	}
};

// ======================================
// This class is responsible for providing Physics related APIs
// ======================================
public ref class PhysicsAPI {
public:
	static System::Nullable<RayCastResult> Raycast(Vector3 origin, Vector3 directionVector, float maxDistance) {
		return Raycast(Ray{ origin, directionVector }, maxDistance, {});
	}

	static System::Nullable<RayCastResult> Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, GameObject^ entityToIgnore) {
		return Raycast(Ray{ origin, directionVector }, maxDistance, entityToIgnore);
	}

	static System::Nullable<RayCastResult> Raycast(Ray^ p_ray, float maxDistance) {
		PhysicsRay ray{ p_ray->native() };
		auto opt = Interface::engine->physicsManager.rayCast(ray, maxDistance);

		if (!opt) {
			return {}; // returns null, no ray cast..
		}

		return System::Nullable<RayCastResult>(RayCastResult{ opt.value() });
	}

	static System::Nullable<RayCastResult> Raycast(Ray^ p_ray, float maxDistance, GameObject^ entityToIgnore) {
		PhysicsRay ray{ p_ray->native() };
		auto opt = Interface::engine->physicsManager.rayCast(ray, maxDistance, { static_cast<entt::entity>(entityToIgnore->entityID) });

		if (!opt) {
			return {}; // returns null, no ray cast..
		}

		return System::Nullable<RayCastResult>(RayCastResult{ opt.value() });
	}
};

// ======================================
// This class is responsible for providing Camera related APIs
// ======================================
public ref class CameraAPI {
public:
	static Ray getRayFromMouse() {
		auto ray = Interface::engine->physicsManager.getRayFromMouse();
		return Ray{ ray };
	}

	static void LockMouse() {
		Interface::engine->gameLockMouse(true);
	}

	static void UnlockMouse() {
		Interface::engine->gameLockMouse(false);
	}
};


// ======================================
// This class is responsible for providing Navigation related APIs to the script.
// ======================================
public ref class NavigationAPI {
public:
	static bool setDestination(GameObject^ gameObject, Vector3^ targetPosition) {
		return Interface::engine->navigationSystem.setDestination(Convert(gameObject), targetPosition->native());
	}
};
// ======================================
// This class is responsible for math related functionality, espeically cause some require conversion to double for some reason
// ======================================
public ref class Mathf {
public:
	// C# doesn't support cos/sin with floats without conversion
	static float Cos(float radian) { return std::cos(radian); }
	static float Sin(float radian) { return std::sin(radian); }
	static float Atan2(float y, float x) { return std::tan(y / x); }
	static float Clamp(float value, float min, float max) { return std::clamp(value, min, max); }
	static float Interpolate(float a, float b, float t, float degree) { return Interpolation::Interpolation(a, b, t, degree); }
	static float Min(float a, float b) { return std::min(a, b); }
	static float Max(float a, float b) { return std::max(a, b); }
public:
	static float Rad2Deg = 360.f/(std::numbers::pi_v<float> * 2);
	static float Deg2Rad = (std::numbers::pi_v<float> *2) / 360.f;
};

// ======================================
// This class is responsible for providing game object creation related APIs.
// ======================================
public ref class ObjectAPI {
public:
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quartenion^ localRotation, GameObject^ parent);
};
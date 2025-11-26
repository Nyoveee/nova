#pragma once

#include "Logger.h"
#include "ManagedTypes.hxx"
#include "API/IManagedComponent.hxx"
#include "API/ConversionUtils.hxx"
#include "API/ScriptingAPI.hxx"
#include "InputManager/inputManager.h"
#include "Interpolation.h"
#include "RandomRange.h"

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
	static float V_FixedDeltaTime(); 
	static float V_DeltaTime();

	static float V_FixedDeltaTime_Unscaled();
	static float V_DeltaTime_Unscaled();

	static property float timeScale {
		float get() { return Interface::engine->deltaTimeMultiplier; };
		void set(float value) { Interface::engine->deltaTimeMultiplier = value; };
	};
};

public ref class Debug {
public:
	static void Log(IManagedComponent^ component);
	static void Log(System::String^ string);
	static void Log(Object^ object);

	static void LogWarning(IManagedComponent^ component);
	static void LogWarning(System::String^ string);
	static void LogWarning(Object^ object);

	static void LogError(IManagedComponent^ component);
	static void LogError(System::String^ string);
	static void LogError(Object^ object);
};

// ======================================
// This class is responsible for providing input related APIs to the script.
// ======================================

public ref class Input {
internal:
	// This functions are called by the script's member function.. this is because each script needs to know 
	// what it has subscribed to for proper destruction.
	static std::size_t MapKey(Key key, EventCallback^ pressCallback);
	static std::size_t MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback);
	static std::size_t MouseMoveCallback(MouseEventCallback^ callback);
	static std::size_t ScrollCallback(ScrollEventCallback^ callback);
	
	static Vector2 V_MousePosition();

internal:
	static void ClearAllKeyMapping();

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
	static void PlaySound(GameObject^ gameObject, System::String^ string);
	static void PlayBGM(GameObject^ gameObject, System::String^ string);
	static void StopSound(GameObject^ gameObject, System::String^ string);
};

// ======================================
// This class is responsible for providing Physics related APIs
// ======================================
public ref class PhysicsAPI {
public:
	static System::Nullable<RayCastResult> Raycast(Vector3 origin, Vector3 directionVector, float maxDistance);
	static System::Nullable<RayCastResult> Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, array<System::String^>^ layermask);
	static System::Nullable<RayCastResult> Raycast(Vector3 origin, Vector3 directionVector, float maxDistance, GameObject^ entityToIgnore);
	static System::Nullable<RayCastResult> Raycast(Ray^ p_ray, float maxDistance);
	static System::Nullable<RayCastResult> Raycast(Ray^ p_ray, float maxDistance, GameObject^ entityToIgnore);
	static System::Nullable<RayCastResult> Linecast(Vector3 start, Vector3 end);
	static System::Nullable<RayCastResult> Linecast(Vector3 start, Vector3 end, array<System::String^>^ layermask);
};

// ======================================
// This class is responsible for providing Camera related APIs
// ======================================
public ref class CameraAPI {
public:
	// Get a ray that points to the mouse from the camera origin..
	// @TODO: Clarify mouse position when its locked.	
	static Ray getRayFromMouse();

	static void LockMouse();
	static void UnlockMouse();
};


// ======================================
// This class is responsible for providing Navigation related APIs to the script.
// ======================================
public ref class NavigationAPI {
public:
	static bool setDestination(GameObject^ gameObject, Vector3^ targetPosition);
	static void stopAgent(GameObject^ gameObject);
};

// ======================================
// This class is responsible for math related functionality, especially cause some require conversion to double for some reason
// ======================================
public ref class Mathf {
public:
	// C# doesn't support cos/sin with floats without conversion
	static float Cos(float radian);
	static float Sin(float radian);
	static float Atan2(float y, float x);
	static float Clamp(float value, float min, float max);
	static float Interpolate(float a, float b, float t, float degree);
	static float Min(float a, float b);
	static float Max(float a, float b);
	static float Pow(float base, float exponent);
	static float Sqrt(float f);
	static float abs(float f);
public:
	static float Rad2Deg = 360.f/(std::numbers::pi_v<float> * 2);
	static float Deg2Rad = (std::numbers::pi_v<float> *2) / 360.f;
};
// ======================================
// This class is responsible for random functionality
// ======================================
public ref class Random {
public:
	static float Range(float minInclusive, float maxInclusive);
	static int Range(int minInclusive, int maxExclusive);
};


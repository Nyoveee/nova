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
	static float V_AccumulatedTime();

	static float V_AccumulatedTime_Unscaled();
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
public:
	static Vector2 GetUIMousePosition();

internal:
	// This functions are called by the script's member function.. this is because each script needs to know 
	// what it has subscribed to for proper destruction.
	static std::size_t MapKey(Key key, EventCallback^ pressCallback, bool toExecuteEvenWhenPaused);
	static std::size_t MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback, bool toExecuteEvenWhenPaused);
	static std::size_t MouseMoveCallback(MouseEventCallback^ callback, bool toExecuteEvenWhenPaused);
	static std::size_t ScrollCallback(ScrollEventCallback^ callback, bool toExecuteEvenWhenPaused);
	
	static Vector2 V_MousePosition();

internal:
	static void ClearAllKeyMapping();

private:
	static System::Collections::Generic::List<std::size_t> scriptObserverIds;
	static System::Collections::Generic::List<std::size_t> mouseMoveObserverIds;
	static System::Collections::Generic::List<std::size_t> mouseScrollObserverIds;

};

// ======================================
// This class is responsible for providing Physics related APIs
// ======================================
public ref class PhysicsAPI {
public:
	static float GetGravity();
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

	static void shakeCamera(float duration, float amplication);
};


// ======================================
// This class is responsible for providing Navigation related APIs to the script.
// ======================================
public ref class NavigationAPI {
public:
	static bool setDestination(GameObject^ gameObject, Vector3^ targetPosition);
	static System::Nullable<Vector3> SampleNavMeshPosition(System::String^ agentMeshName, Vector3^ sourcePosition, Vector3^ halfExtent);

	//Suggest to use Sample NavMeshPosition if to test for startPosition and End Position
	static  System::Collections::Generic::List<Vector3>^ CalculatePath(System::String^ agentMeshName, Vector3^ startPosition, Vector3^ endPosition);
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
	static float Abs(float value);
	static float SmoothLerp(float a, float b, float t);

public:
	static float Rad2Deg = 360.f/(std::numbers::pi_v<float> * 2);
	static float Deg2Rad = (std::numbers::pi_v<float> *2) / 360.f;
};

// ======================================
// This class is responsible for random functionality
// ======================================
public ref class Random {
public:
	static Vector3 Range(Vector3 minInclusive, Vector3 maxInclusive);
	static float Range(float minInclusive, float maxInclusive);
	static int Range(int minInclusive, int maxExclusive);
};

// ======================================
// This class is responsible for scene management
// ======================================
public ref class SceneAPI {
public:
	static void ChangeScene(ScriptingAPI::Scene^ sceneId);
};

// ======================================
// This class for providing system control
// ======================================
public ref class Systems {
public:
	static property bool Pause {
		bool get() { return Interface::engine->isPaused; };
		void set(bool value) { Interface::engine->pauseSystems(value); };
	};
	static property Vector2 ScreenResolution {
		Vector2 get() { return Vector2(static_cast<float>(Interface::engine->getGameWidth()), static_cast<float>(Interface::engine->getGameHeight())); };
	}

	static void Restart();
	static void Quit();
};

public ref class PlayerPrefs {
public:
	static float			GetFloat(System::String^ key);
	static int				GetInt(System::String^ key);
	static System::String^  GetString(System::String^ key);

	static float			GetFloat(System::String^ key, float defaultValue);
	static int				GetInt(System::String^ key, int defaultValue);
	static System::String^	GetString(System::String^ key, System::String^ defaultValue);

	static void				SetFloat(System::String^ key, float value);
	static void				SetInt(System::String^ key, int value);
	static void				SetString(System::String^ key, System::String^ value);

	static void				Save();
	static void				DeleteKey(System::String^ key);
	static void				DeleteAll();
};

public ref class RendererAPI {
public:
#if 0
	static property bool toneMapping {
		bool get() { return Interface::engine->renderer.toneMappingMethod == Renderer::ToneMappingMethod::ACES; };
		void set(bool value) { Interface::engine->renderer.toneMappingMethod = value ? Renderer::ToneMappingMethod::ACES : Renderer::ToneMappingMethod::None; };
	};

#endif
	static property bool toPostProcess {
		bool get() { return Interface::engine->renderer.toPostProcess; };
		void set(bool value) { Interface::engine->renderer.toPostProcess = value; };
	};

	static property float exposure {
		float get() { return Interface::engine->renderer.hdrExposure; };
		void set(float value) { Interface::engine->renderer.hdrExposure = value; };
	};

	static property float vignette {
		float get() { return Interface::engine->renderer.vignette; };
		void set(float value) { Interface::engine->renderer.vignette = value; };
	};

	static property bool ssaoEnabled {
		bool get() { return Interface::engine->dataManager.renderConfig.toEnableSSAO; };
		void set(bool value) { Interface::engine->dataManager.renderConfig.toEnableSSAO = value; };
	}
	static property bool fogEnabled {
		bool get() { return Interface::engine->dataManager.renderConfig.toEnableFog; };
		void set(bool value) { Interface::engine->dataManager.renderConfig.toEnableFog = value; };
	}
	static property bool antiAliasingEnabled {
		bool get() { return Interface::engine->dataManager.renderConfig.toEnableAntiAliasing; };
		void set(bool value) { Interface::engine->dataManager.renderConfig.toEnableAntiAliasing = value; };
	}
	static property bool shadowsEnabled {
		bool get() { return Interface::engine->dataManager.renderConfig.toEnableShadows; };
		void set(bool value) { Interface::engine->dataManager.renderConfig.toEnableShadows = value; };
	}
	static property bool iblEnabled {
		bool get() { return Interface::engine->dataManager.renderConfig.toEnableIBL; };
		void set(bool value) { Interface::engine->dataManager.renderConfig.toEnableIBL = value; };
	}
	static property bool fullScreen {
		bool get() { return Interface::engine->dataManager.renderConfig.fullScreen; };
		void set(bool value) { Interface::engine->setFullscreen(value); };
	}
};

// ======================================
// This class is responsible for providing audio related APIs to the script.
// ======================================
public ref class AudioAPI {
public:
	static void SetMasterVolume(float volume)	{ Interface::engine->audioSystem.setMasterVolume(volume); Interface::engine->dataManager.audioConfig.masterVolume = volume; };
	static void SetBGMVolume(float volume)		{ Interface::engine->audioSystem.setBGMVolume(volume); Interface::engine->dataManager.audioConfig.bgmVolume = volume; };
	static void SetSFXVolume(float volume)		{ Interface::engine->audioSystem.setSFXVolume(volume); Interface::engine->dataManager.audioConfig.sfxVolume = volume; };

	static float GetMasterVolume()	{ return Interface::engine->dataManager.audioConfig.masterVolume; };
	static float GetBGMVolume()		{ return Interface::engine->dataManager.audioConfig.bgmVolume; };
	static float GetSFXVolume()		{ return Interface::engine->dataManager.audioConfig.sfxVolume; };
};

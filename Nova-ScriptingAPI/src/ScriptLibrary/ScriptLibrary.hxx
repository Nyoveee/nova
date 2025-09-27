#include "Logger.h"
#include "ManagedTypes.hxx"
#include "API/IManagedComponent.hxx"
#include "API/ConversionUtils.h"
#include "API/ScriptingAPI.hxx"
#include "InputManager/inputManager.h"
/*******************************************************************************************
	For this library, you would need to understand the difference between
	Managed Types(For C#) and Native Types(C++)

	To Create Managed Type Versions based on Native Versions, go to ManagedTypes.hxx

 	For Variables Indicate with V_

	Conversion from Managed to native types is provided:
		Component: Convert(Managed Component)
		Managed Struct: VariableName->native()
		Primitives: Converted by default for these types https://learn.microsoft.com/en-us/cpp/dotnet/managed-types-cpp-cli?view=msvc-170
		Strings: Convert(Managed String)
		Event Callbacks: Convert<Event>(Managed Callback,key)

	Recommended to only have one line which is to simply call the engine function
*******************************************************************************************/
public ref class Time {
public:
	static float V_FixedDeltaTime() { return 1 / 60.f; } // Replace with config
};

public ref class Debug {
public:
	static void Print(IManagedComponent^ component) { Logger::info(Convert(component->ToString()));}
	static void Print(System::String^ string)		{ Logger::info(Convert(string)); }
	static void Print(Object^ object)               { Logger::info(Convert(object->GetType()->Name) + Convert(object->ToString())); }
};

// ======================================
// This class is responsible for providing input related APIs to the script.
// ======================================

public ref class Input {
public:
	static void MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback) { 
		Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key), Convert<ScriptingInputEvents>(releaseCallback, key));
	}
	static Vector2 V_MousePosition() { return Vector2(Interface::engine->inputManager.mousePosition); }
	static float V_ScrollOffsetY() { return Interface::engine->inputManager.scrollOffsetY; }
};

// ======================================
// This class is responsible for providing audio related APIs to the script.
// ======================================
public ref class AudioAPI {
public:
	static void playSound(unsigned int entityId, System::String^ string) {
		Interface::engine->audioSystem.playSFX(static_cast<entt::entity>(entityId), Convert(string));
	}
};


// ======================================
// This class is responsible for providing Navigation related APIs to the script.
// ======================================
public ref class NavigationAPI {
public:
	static bool setDestination(unsigned int entityId, Vector3^ targetPosition) {
		return Interface::engine->navigationSystem.setDestination(static_cast<entt::entity>(entityId), targetPosition->native());
	}
};
#include "Logger.h"
#include "ManagedTypes.hxx"
#include "API/IManagedComponent.hxx"
#include "API/ConversionUtils.h"
#include "API/ScriptingAPI.hxx"
#include "InputManager/inputManager.h"
/*******************************************************************************************
 	For Variables Indicate with V_

	Conversion to native types:
		Component: To Do
		Managed Struct: VariableName->Native()
		Primitives: Converted by default for these types https://learn.microsoft.com/en-us/cpp/dotnet/managed-types-cpp-cli?view=msvc-170
		Additional Methods to convert to native types can be found in ConversionUtils.h

	Recommended to only have one line which is to call the engine function
*******************************************************************************************/
public ref class Time {
public:
	static float V_FixedDeltaTime() { return 1 / 60.f; } // Replace with config
};

public ref class Debug {
public:
	static void Print(IManagedComponent^ component) { Logger::info(Convert(component->ToString()));}
	static void Print(Object^ object)               { Logger::info(Convert(object->GetType()->Name) + Convert(object->ToString())); }
};

public ref class Input {
public:
	static void MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback) { 
		Interface::engine->inputManager.subscribe(Convert<ScriptingInputEvents>(pressCallback, key), Convert<ScriptingInputEvents>(releaseCallback, key));
	}
	static Vector2 V_MousePosition() { return Vector2(Interface::engine->inputManager.mousePosition); }
	static float V_ScrollOffsetY() { return Interface::engine->inputManager.scrollOffsetY; }
};
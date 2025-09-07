// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <string>

class Engine;
class ECS;
ref class Script;
ref class IManagedComponent;

public ref class Interface
{
public:
	using ScriptID = System::UInt64;
	using EntityID = System::UInt32;

// This is set to internal so c# scripts cannot access
internal:
	static void init(Engine& p_engine, const char* p_runtimeDirectory);
	static void load();
	static void unload();
	static void update();

	static void addGameObjectScript(EntityID entityID, ScriptID scriptId);
	static void removeGameObjectScript(EntityID entityID, ScriptID scriptId);
	static void intializeAllScripts();

internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);
internal:
	static Engine* engine;

private:
	using Scripts = System::Collections::Generic::List<Script^>;
	using Components = System::Collections::Generic::List<IManagedComponent^>;

	// Stores all of the loaded scripts of a given game object.
	static System::Collections::Generic::Dictionary<EntityID, Scripts^>^ gameObjectScripts;
	
	// Store all unique script type. To be used for instantiation.
	// We map an Asset ID to the corresponding script type.
	static System::Collections::Generic::Dictionary<ScriptID, System::Type^>^ scriptTypes;

	// Assembly information
	static System::Runtime::Loader::AssemblyLoadContext^ assemblyLoadContext;
	static const char* runtimePath;

	// Script Checking Information
	System::IO::FileSystemWatcher^ scriptWatcher;
}; 
#include "ScriptingAPI.ixx"

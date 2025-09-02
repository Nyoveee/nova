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
// This is set to internal so c# scripts cannot access
internal:
	static void init(Engine& p_engine, const char* p_runtimeDirectory);
	static void update();
	static void hotReload();
	static void addGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
	static void removeGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
	static std::vector<std::string> getScriptNames();
internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);
internal:
	static Engine* engine;
private:
	using Scripts = System::Collections::Generic::List<Script^>;
	using Components = System::Collections::Generic::List<IManagedComponent^> ;
	static System::Collections::Generic::Dictionary<System::UInt32, Scripts^>^ gameObjectScripts;
	static System::Collections::Generic::List<System::Type^> scriptTypes;
private:
	// Assembly information
	static System::Runtime::Loader::AssemblyLoadContext^ assemblyLoadContext;
	static const char* runtimePath;
}; 
#include "ScriptingAPI.ixx"

// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <string>

class ECS;
ref class Script;
ref class IManagedComponent;

public ref class Interface
{
// This is set to internal so c# scripts cannot access
internal:
	static void init(ECS& ecs, const char* runtimePath);
	static void update();
	static void addGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
	static void removeGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
	static std::vector<std::string> getScriptNames();
internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);
private:
	static entt::registry* registry;
	using Scripts = System::Collections::Generic::List<Script^>;
	using Components = System::Collections::Generic::List<IManagedComponent^> ;
	static System::Collections::Generic::Dictionary<System::UInt32, Scripts^>^ gameObjectScripts;
	static System::Collections::Generic::List<System::Type^> scriptTypes;
}; 
#include "ScriptingAPI.ixx"

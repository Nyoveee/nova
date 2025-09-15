// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <string>
#include "Engine/engine.h"

class Engine;
class ECS;
ref class Script;
ref class IManagedComponent;

public ref class Interface
{
public:
	using ScriptID = System::UInt64;
	using EntityID = System::UInt32;
internal:
	static void init(Engine& p_engine, const char* p_runtimeDirectory);
	static void load();
	static void unload();
	static void update();

	static void addGameObjectScript(EntityID entityID, ScriptID scriptId);
	static void removeGameObjectScript(EntityID entityID, ScriptID scriptId);
	static void clearAllScripts();
	static void intializeAllScripts();
	static std::vector<FieldData> getScriptFieldDatas(EntityID entityID, ScriptID scriptID);
	//static void setScriptData(EntityID entityID, ScriptID scriptId, const char* name, void* value);

internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);
	// Append Primitive data value to the fielddata
	template<typename Type, typename ...Types>
	static bool AppendPrimitiveData(FieldData& fieldData, Object^ object);
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
}; 
#include "ScriptingAPI.ixx"

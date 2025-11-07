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
	static void loadAssembly();
	static void unloadAssembly();

	static void update();

	// static void editorModeUpdate();
	static void addEntityScript(EntityID entityID, ScriptID scriptId);
	static void initializeScript(EntityID entityID, ScriptID scriptId);

	static void removeEntity(EntityID entityID);
	static void removeEntityScript(EntityID entityID, ScriptID scriptId);

	static void intializeAllScripts();

	static void handleOnCollision(EntityID entityOne, EntityID entityTwo);

	// This function is called when a particular function name needs to be invoked.
	static void executeEntityScriptFunction(EntityID entityID, ScriptID scriptId, std::string const& functionName);

	static void submitGameObjectDeleteRequest(EntityID entityToBeDeleted);

internal:
	// Script Fields
	static std::vector<FieldData> getScriptFieldDatas(ScriptID scriptID);

	// Set script field data..
	static void setScriptFieldData(EntityID entityID, ScriptID scriptID, FieldData const& fieldData);

internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);

	// Setting/Getting of primitive data for fields through fielddata
	template<typename Type, typename ...Types>
	static bool ObtainPrimitiveDataFromScript(FieldData& fieldData, Object^ object);

	template<typename Type, typename ...Types>
	static bool SetScriptPrimitiveFromNativeData(FieldData const& fieldData,Script^ script, System::Reflection::FieldInfo^ fieldInfo);

	template <typename Type, typename ...Types>
	static bool ObtainTypedResourceIDFromScript(FieldData& fieldData, Object^ object, System::Type^ originalType);

	template <typename Type, typename ...Types>
	static bool SetTypedResourceIDFromScript(FieldData const& fieldData, Script^ script, System::Reflection::FieldInfo^ fieldInfo);

private:
	// static void updateReference(Script^ script);

internal:
	static Engine* engine;

private:
	using Scripts = System::Collections::Generic::Dictionary<ScriptID, Script^>;
	using Components = System::Collections::Generic::List<IManagedComponent^>;

	// Stores all of the loaded scripts of a given game object.
	static System::Collections::Generic::Dictionary<EntityID, Scripts^>^ gameObjectScripts;
	
	// Store all unique script type. To be used for instantiation.
	// We map an Asset ID to the corresponding script type.
	static System::Collections::Generic::Dictionary<ScriptID, Script^>^ availableScripts;

	// Stores all the game object that is requested to be deleted. We delay object destruction till the end of the frame.
	// (had bad experience with instant deletion..)
	static System::Collections::Generic::Queue<EntityID> deleteGameObjectQueue;

	// Assembly information
	static System::Runtime::Loader::AssemblyLoadContext^ assemblyLoadContext;
	static const char* runtimePath;
}; 
#include "ScriptingAPI.ixx"

// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <string>
#include "Engine/engine.h"

#include "TimeoutDelegate.hxx"

class Engine;
class ECS;
ref class Script;
ref class IManagedComponent;

namespace ScriptingAPI {
	ref class Scene;
}

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
	static void fixedUpdate();

	static void addEntityScript(EntityID entityID, ScriptID scriptId);
	static Script^ delayedAddEntityScript(EntityID entityID, ScriptID scriptId);

	static void intializeAllScripts();
	static void initializeScript(Script^ script);

	static void removeEntity(EntityID entityID);
	static void removeEntityScript(EntityID entityID, ScriptID scriptId);

	static void handleOnCollision(EntityID entityOne, EntityID entityTwo);
	static void handleOnCollisionExit(EntityID entityOne, EntityID entityTwo);

	// This function is called when a particular function name needs to be invoked.
	static void executeEntityScriptFunction(EntityID entityID, ScriptID scriptId, std::string const& functionName);

	static void submitGameObjectDeleteRequest(EntityID entityToBeDeleted);

	static void changeSceneRequest(ScriptingAPI::Scene^ newScene);

	static void recursivelyInitialiseEntity(entt::entity entity);

internal:
	// Script Fields
	static std::vector<FieldData> getScriptFieldDatas(ScriptID scriptID);
	
	// process each individual script field data.. and get info from it..
	static bool getScriptFieldData(System::Object^ object, System::Type^ fieldType, serialized_field_type& fieldData);

	// Set script field data..
	static void setScriptFieldData(EntityID entityID, ScriptID scriptID, FieldData const& fieldData);

	// Used when instantiating a new script..
	static void setFieldData(Script^ script, FieldData const& fieldData);

	// process each individual script field data.. and set to it..
	static void processSetScriptFieldData(Object^% object, System::Type^ fieldType, serialized_field_type const& field);

	static void addTimeoutDelegate(TimeoutDelegate^ timeoutDelegate);

	static std::unordered_set<ResourceID> GetHierarchyModifiedScripts(ScriptID scriptId);

	static std::vector<std::string> getEnumNames(System::String^ type);

internal:
	template<typename T>
	static T* getNativeComponent(System::UInt32 entityID);
	
	generic<typename T> where T : Script
	static T tryGetScriptReference(System::UInt32 entityID);

	// Setting/Getting of primitive data for fields through fielddata
	template<typename Type, typename ...Types>
	static bool ObtainPrimitiveDataFromScript(serialized_field_type& fieldData, Object^ object);

	template<typename Type, typename ...Types>
	static bool SetScriptPrimitiveFromNativeData(serialized_field_type const& fieldData, Object^% object);

	template <typename Type, typename ...Types>
	static bool ObtainTypedResourceIDFromScript(serialized_field_type& fieldData, Object^ object, System::Type^ originalType);

	template <typename Type, typename ...Types>
	static bool SetTypedResourceIDFromScript(serialized_field_type const& fieldData, Object^% object);

internal:
	static Engine* engine;

private:
	using ScriptDictionary = System::Collections::Generic::Dictionary<ScriptID, Script^>;

	using Components = System::Collections::Generic::List<IManagedComponent^>;

	// Stores all of the loaded scripts of a given game object.
	static System::Collections::Generic::Dictionary<EntityID, ScriptDictionary^>^ gameObjectScripts;
	
	// Store all unique script type. To be used for instantiation.
	// We map an Asset ID to the corresponding script type.
	static System::Collections::Generic::Dictionary<ScriptID, Script^>^ availableScripts;
	static System::Collections::Generic::Dictionary<ScriptID, System::Type^>^ abstractScriptTypes;

	// Map enum names to values and types
	static System::Collections::Generic::Dictionary<System::String^, array<System::String^>^>^ enumTypeNamesToValues;
	static System::Collections::Generic::Dictionary<System::String^, System::Type^>^ enumTypes;

	// Stores all the game object that is requested to be deleted. We delay object destruction till the end of the frame.
	// (had bad experience with instant deletion..)
	static System::Collections::Generic::Queue<EntityID> deleteGameObjectQueue;

	// We store created game object scripts in a separate dictionary first..
	static System::Collections::Generic::Dictionary<EntityID, ScriptDictionary^>^ createdGameObjectScripts;

	// We store set timeout request in a separate container..
	static System::Collections::Generic::List<TimeoutDelegate^>^ timeoutDelegates;

	// Temporary container to execute and remove it from the main container..
	static System::Collections::Generic::List<TimeoutDelegate^>^ executeTimeoutDelegates;

	// We delay change scene request, by storing it in a variable first.
	static ScriptingAPI::Scene^ newSceneToChangeTo;

	// Assembly information
	static System::Runtime::Loader::AssemblyLoadContext^ assemblyLoadContext;
	static const char* runtimePath;
}; 
#include "ScriptingAPI.ixx"

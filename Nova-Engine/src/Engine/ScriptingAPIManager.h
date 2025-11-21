/*
	Reference:
	https://kahwei.dev/tag/scripting/
*/

#pragma once
#include "export.h"
#include "type_alias.h"

#include <entt/entt.hpp>
#include <dotnet/coreclrhost.h>

#define NOMINMAX
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <unordered_set>

#include "Physics/physicsManager.h"

struct FieldData;

// More readable function pointer syntax because C lmaoo
using UpdateFunctionPtr				         = void (*)(void);
using AddScriptFunctionPtr			         = void (*)(unsigned int, std::size_t);
using RemoveScriptFunctionPtr		         = void (*)(unsigned int, std::size_t);
using RemoveEntityFunctionPtr                = void (*)(unsigned int);
using LoadScriptsFunctionPtr		         = void (*)(void);
using UnloadScriptsFunctionPtr	 	         = void (*)(void);
using IntializeScriptsFunctionPtr	         = void (*)(void);
using GetScriptFieldsFunctionPtr             = std::vector<FieldData> (*)(std::size_t);
using SetScriptFieldFunctionPtr		         = void (*)(unsigned int, unsigned long long, FieldData const& fieldData);

using handleOnCollisionFunctionPtr		     = void (*)(unsigned int, unsigned int);
using ExecuteFunctionPtr			         = void (*)(unsigned int, unsigned long long, std::string const&);
using GetHierarchyModifiedScriptsFunctionPtr = std::unordered_set<ResourceID>(*)(std::size_t);
class Engine;

class ScriptingAPIManager {
public:
	enum class CompileState {
		NotCompiled,
		ToBeCompiled,
		Compiled,
		CompilationFailed
	};

public:
	// Functions needed to run the ScriptingAPI
	ENGINE_DLL_API ScriptingAPIManager(Engine& p_engine);

	ENGINE_DLL_API ~ScriptingAPIManager();
	ENGINE_DLL_API ScriptingAPIManager(ScriptingAPIManager const& other)				= delete;
	ENGINE_DLL_API ScriptingAPIManager(ScriptingAPIManager&& other)						= delete;
	ENGINE_DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager const& other)		= delete;
	ENGINE_DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager&& other)			= delete;

public:
	ENGINE_DLL_API bool compileScriptAssembly();

	ENGINE_DLL_API bool isNotCompiled() const;
	ENGINE_DLL_API bool hasCompilationFailed() const;

	// Simulation
	ENGINE_DLL_API bool startSimulation();
	ENGINE_DLL_API void stopSimulation();

	// Update
	ENGINE_DLL_API void update();
	ENGINE_DLL_API void checkIfRecompilationNeeded(float dt);

	// Serializable Field Reference
	ENGINE_DLL_API std::vector<FieldData> getScriptFieldDatas(ResourceID scriptID);

	//ENGINE_DLL_API bool setScriptFieldData(entt::entity entityID, ResourceID scriptID, FieldData const& fieldData);

	// This is the callback when the assets files are Added
	ENGINE_DLL_API void OnAssetContentAddedCallback(std::string abspath);

	// This is the callback when the assets files are Modified
	ENGINE_DLL_API void OnAssetContentModifiedCallback(ResourceID assetTypeID);

	// This is the callback when the assets files are deleted
	ENGINE_DLL_API void OnAssetContentDeletedCallback(ResourceID assetTypeID);

public:
	// Interfaces
	ENGINE_DLL_API void onCollisionEnter(entt::entity entityOne, entt::entity entityTwo);
	ENGINE_DLL_API void onCollisionExit(entt::entity entityOne, entt::entity entityTwo);

	ENGINE_DLL_API void executeFunction(entt::entity entityOne, ResourceID scriptID, std::string const& functionName);

private:
	template<typename Func>
	Func getCoreClrFuncPtr(const std::string& functionName);

	// Get Function from ScriptingAPI
	template<typename Func>
	Func GetFunctionPtr(std::string typeName, std::string functionName);

	std::string buildTPAList(const std::string& directory);
	std::string getDotNetRuntimeDirectory();


private:
	Engine& engine;
	std::string runtimeDirectory;

private:
	// coreCLR key components 
	HMODULE coreClr;
	void* hostHandle;
	unsigned int domainID;

	// coreCLR functions
	coreclr_initialize_ptr intializeCoreClr;
	coreclr_create_delegate_ptr createManagedDelegate;
	coreclr_shutdown_ptr shutdownCorePtr;

private:
	// Function pointers to interact directly with ScriptingAPI
	UpdateFunctionPtr						 update_;
	AddScriptFunctionPtr					 addEntityScript;
	RemoveScriptFunctionPtr					 removeEntityScript_;
	RemoveEntityFunctionPtr					 removeEntity_;
	LoadScriptsFunctionPtr					 loadAssembly;
	UnloadScriptsFunctionPtr				 unloadAssembly;
	IntializeScriptsFunctionPtr				 initalizeScripts;
	GetScriptFieldsFunctionPtr				 getScriptFieldDatas_;
	SetScriptFieldFunctionPtr				 setScriptFieldData;
	handleOnCollisionFunctionPtr			 handleOnCollision_;
	ExecuteFunctionPtr				         executeFunction_;
	GetHierarchyModifiedScriptsFunctionPtr   getHierarchyModifiedScripts_;
private:
	CompileState compileState;
	float timeSinceSave;

	// we keep track of only the scripts that has been modified when recompiling, and reload the 
	// default values of the serialized fields for those affected scripts.
	std::unordered_set<ResourceID> modifiedScripts;
};

#include "Engine/ScriptingAPIManager.ipp"
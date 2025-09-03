/*
	Reference:
	https://kahwei.dev/tag/scripting/
*/

#pragma once
#include "export.h"

#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <dotnet/coreclrhost.h>

class Engine; // Don't need to call any function or variables, just pass in

// More readable function pointer syntax because C lmaoo
using UpdateFunctionPtr			= void (*)(void);
using AddScriptFunctionPtr		= void (*)(unsigned int, const char*);
using RemoveScriptFunctionPtr	= void (*)(unsigned int, const char*);
using LoadScriptsFunctionPtr = void (*)(void);
using UnloadScriptsFunctionPtr = void(*)(void);

class ScriptingAPIManager {
public:
	// Functions needed to run the ScriptingAPI
	DLL_API ScriptingAPIManager(Engine& p_engine);

	DLL_API ~ScriptingAPIManager();
	DLL_API ScriptingAPIManager(ScriptingAPIManager const& other)				= delete;
	DLL_API ScriptingAPIManager(ScriptingAPIManager&& other)					= delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager const& other)	= delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager&& other)			= delete;

public:
	DLL_API void update();

	DLL_API bool loadAllScripts();
	DLL_API void unloadAllScripts();

	DLL_API void loadScriptIntoAPI(unsigned int entityID, const char* scriptName);
	DLL_API void removeScriptFromAPI(unsigned int entityID, const char* scriptName);


private:
	template<typename Func>
	Func getCoreClrFuncPtr(const std::string& functionName);

	// Get Function from ScriptingAPI
	template<typename Func>
	Func GetFunctionPtr(std::string typeName, std::string functionName);

	std::string buildTPAList(const std::string& directory);
	std::string getDotNetRuntimeDirectory();
	bool compileScriptAssembly();
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
	UpdateFunctionPtr updateScripts;
	AddScriptFunctionPtr addGameObjectScript;
	RemoveScriptFunctionPtr removeGameObjectScript;
	LoadScriptsFunctionPtr loadScripts;
	UnloadScriptsFunctionPtr unloadScripts;
};

#include "Engine/ScriptingAPIManager.ipp"
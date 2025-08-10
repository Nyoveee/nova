/*
	Reference:
	https://kahwei.dev/tag/scripting/
*/

#pragma once
#include "export.h"

#include <Windows.h>
#include <sstream>
#include <iomanip>

#include "../Include/dotnet/coreclrhost.h"
// Maybe make this follow the system in engine

class Engine; // Don't need to call any function or variables, just pass in

class ScriptingAPIManager {
public:
	// Functions needed to run the ScriptingAPI
	DLL_API ScriptingAPIManager(Engine& engine);
	DLL_API ~ScriptingAPIManager();
	DLL_API ScriptingAPIManager(ScriptingAPIManager const& other) = delete;
	DLL_API ScriptingAPIManager(ScriptingAPIManager&& other) = delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager const& other) = delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager&& other) = delete;
	DLL_API void update();
	// Adding removing scripts into the api
	DLL_API void loadScriptIntoAPI(unsigned int entityID, const char* scriptName);
private:
	// coreCLR key components 
	HMODULE coreClr;
	void* hostHandle;
	unsigned int domainID;

	// coreCLR functions
	coreclr_initialize_ptr intializeCoreClr;
	coreclr_create_delegate_ptr createManagedDelegate;
	coreclr_shutdown_ptr shutdownCorePtr;

	// ScriptingAPI functions
	void(*updateScripts)(void);
	void(*addGameObjectScript)(unsigned int, const char*);
	void(*removeGameObjectScript)(unsigned int, const char*);

	std::string buildTPAList(const std::string& directory);

	template<typename Func>
	Func getCoreClrFuncPtr(const std::string& functionName) {
		// Get function from dll
		Func fptr = reinterpret_cast<Func>(GetProcAddress(coreClr, functionName.c_str()));
		if (!fptr)
			throw std::runtime_error("Unable to get pointer to function.");
		return fptr;
	}

	// Get Function from ScriptingAPI
	template<typename Func>
	Func GetFunctionPtr(std::string assemblyName, std::string typeName, std::string functionName) {
		Func managedDelegate{ nullptr };
		int result = createManagedDelegate(
			hostHandle,
			domainID,
			assemblyName.data(),
			typeName.data(),
			functionName.data(),
			reinterpret_cast<void**>(&managedDelegate)
		);
		if (result != S_OK) {
			std::ostringstream errorDetails;
			errorDetails << "(0x";
			errorDetails << std::hex << result;
			errorDetails << std::string{ ") Unable to Get Function " + assemblyName + '|' + typeName + '|' + functionName }.c_str();
			throw std::runtime_error(errorDetails.str());
		}
		return managedDelegate;
	}
};
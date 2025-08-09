/*
	Reference:
	https://kahwei.dev/tag/scripting/
*/

#pragma once

#include <Windows.h>
#include <sstream>
#include <iomanip>

#include "../Include/dotnet/coreclrhost.h"
// Maybe make this follow the system in engine
class ScriptingAPIManager {
public:
	// Functions needed to run the ScriptingAPI
	ScriptingAPIManager();
	void initializeScriptingAPI();
	void update();
	void stopScriptingAPI();
private:
	std::string buildTPAList(const std::string& directory);

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
	void(*addGameObjectScript)(int, const char*);
	void(*removeGameObjectScript)(int, const char*);

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
#pragma once
#include <string>
#include <Windows.h>
#include <iomanip>

#include "export.h"
#include "../Include/dotnet/coreclrhost.h"


class DLL_API Engine {
public:
	Engine();
	void run();
private:
	// Functions needed to run the ScriptingAPI
	void initializeScriptingAPI();
	std::string buildTPAList(const std::string& directory);
	void stopScriptingAPI();
	// coreCLR key components 
	HMODULE coreClr;
	void* hostHandle;
	unsigned int domainID;

	// coreCLR functions
	coreclr_initialize_ptr intializeCoreClr;
	coreclr_create_delegate_ptr createManagedDelegate;
	coreclr_shutdown_ptr shutdownCorePtr;

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
			errorDetails << std::string{ ") Unable to Get Function " + assemblyName + '|' + typeName + '|' + functionName}.c_str();
			throw std::runtime_error(errorDetails.str());
		}
		return managedDelegate;
	}
};
#include "ScriptingAPIManager.h"

#include <shlwapi.h>
#include <array>
#include <iostream>

#pragma comment(lib, "shlwapi.lib") // PathRemoveFileSpecA

ScriptingAPIManager::ScriptingAPIManager(Engine& engine)
	: coreClr{ nullptr }
	, hostHandle{ nullptr }
	, domainID{}
	, intializeCoreClr{ nullptr }
	, createManagedDelegate{ nullptr }
	, shutdownCorePtr(nullptr)
	, updateScripts{ nullptr } 
	, addGameObjectScript{ nullptr }
	, removeGameObjectScript{ nullptr } 
{
	// Get the file path of the output directory containing coreclr.dll
	std::string runtimePath{ std::string(MAX_PATH, '\0') };
	GetModuleFileNameA(nullptr, runtimePath.data(), MAX_PATH);
	PathRemoveFileSpecA(runtimePath.data());
	runtimePath.resize(std::strlen(runtimePath.data()));
	std::string coreClrPath{ runtimePath };
	coreClrPath += "\\Coreclr.dll";

	// Load coreclr.dll
	coreClr = LoadLibraryExA(coreClrPath.c_str(), nullptr, 0);
	if (!coreClr)
		throw std::runtime_error("Failed to load CoreCLR.");
	// Get CoreCLR hosting functions
	try {
		intializeCoreClr = getCoreClrFuncPtr<coreclr_initialize_ptr>("coreclr_initialize");
		createManagedDelegate = getCoreClrFuncPtr< coreclr_create_delegate_ptr>("coreclr_create_delegate");
		shutdownCorePtr = getCoreClrFuncPtr<coreclr_shutdown_ptr>("coreclr_shutdown");
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}

	// Construct AppDomain Properties used when starting the runtime
	std::string tpaList{ buildTPAList(runtimePath) };
	std::array propertyKeys{ "TRUSTED_PLATFORM_ASSEMBLIES", "APP_PATHS" };
	std::array propertyValues{ tpaList.c_str(),runtimePath.c_str() };

	// Start CoreClr runtime
	int result = intializeCoreClr(
		runtimePath.c_str(),
		"Nova-Host",
		static_cast<int>(propertyKeys.size()),
		propertyKeys.data(),
		propertyValues.data(),
		&hostHandle,
		&domainID
	);
	if (result != S_OK) {
		std::ostringstream errorDetails;
		errorDetails << "(0x";
		errorDetails << std::hex << result;
		errorDetails << ")Failed to initialize CoreCLR";
		throw std::runtime_error(errorDetails.str());
	}

	// Get the functions to run the api
	try {
		void(*init)(Engine&,const char*) = GetFunctionPtr<void(*)(Engine&,const char*)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "init");
		updateScripts = GetFunctionPtr<void(*)(void)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "update");
		addGameObjectScript = GetFunctionPtr<void(*)(unsigned int, const char*)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "addGameObjectScript");
		removeGameObjectScript = GetFunctionPtr<void(*)(unsigned int, const char*)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "removeGameObjectScript");
		getScriptNames = GetFunctionPtr<std::vector<std::string>(*)(void)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "getScriptNames");
		init(engine,runtimePath.c_str());
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}
}

ScriptingAPIManager::~ScriptingAPIManager()
{
	// Shut down CoreClr
	int result{ shutdownCorePtr(hostHandle,domainID) };
	if (result != S_OK) {
		std::ostringstream errorDetails;
		errorDetails << "(0x";
		errorDetails << std::hex << result;
		errorDetails << ")Failed to shut down CoreCLR";
		std::cout << errorDetails.str() << std::endl;
	}
}

std::string ScriptingAPIManager::buildTPAList(const std::string& directory)
{
	const std::string search_path{ directory + "\\*.dll" };

	std::ostringstream tpaList;

	// Search directory for TPAs(.dll)
	WIN32_FIND_DATAA findData;
	HANDLE fileHandle{ FindFirstFileA(search_path.c_str(), &findData) };
	if (fileHandle != INVALID_HANDLE_VALUE) {
		do {
			tpaList << directory << '\\' << findData.cFileName << ';';
		} while (FindNextFileA(fileHandle, &findData));
		FindClose(fileHandle);
	}
	return tpaList.str();
}
void ScriptingAPIManager::update() { updateScripts(); }
void ScriptingAPIManager::loadScriptIntoAPI(unsigned int entityID, const char* scriptName){ addGameObjectScript(entityID, scriptName); }
void ScriptingAPIManager::removeScriptFromAPI(unsigned int entityID, const char* scriptName) { removeGameObjectScript(entityID, scriptName); }
std::vector<std::string> ScriptingAPIManager::getAvailableScripts(){ return getScriptNames(); }


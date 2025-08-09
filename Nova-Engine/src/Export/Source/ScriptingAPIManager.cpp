#include <shlwapi.h>
#include <array>
#include <filesystem>

#include "../Header/ScriptingAPIManager.h"

ScriptingAPIManager::ScriptingAPIManager()
	: coreClr{ nullptr }
	, hostHandle{ nullptr }
	, domainID{}
	, intializeCoreClr{ nullptr }
	, createManagedDelegate{ nullptr }
	, shutdownCorePtr(nullptr)
	, updateScripts{ nullptr } 
	, addGameObjectScript{ nullptr }
	, removeGameObjectScript{ nullptr } {}


void ScriptingAPIManager::initializeScriptingAPI()
{
	// Get the file path of the output directory containing coreclr.dll
	std::string runtimePath{ std::string(MAX_PATH, '\0') };
	GetModuleFileNameA(nullptr, runtimePath.data(), MAX_PATH);
	PathRemoveFileSpecA(runtimePath.data());
	runtimePath.resize(std::strlen(runtimePath.data()));
	std::string coreClrPath{ runtimePath };
	coreClrPath += "\\Coreclr.dll";

	std::filesystem::current_path(runtimePath);

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
	catch (...) {
		throw;
	}

	// Construct AppDomain Properties used when starting the runtime
	std::string tpaList{ buildTPAList(runtimePath) };
	std::array propertyKeys{ "TRUSTED_PLATFORM_ASSEMBLIES", "APP_PATHS" };
	std::array propertyValues{ tpaList.c_str(),runtimePath.c_str() };

	// Start CoreClr runtime
	int result = intializeCoreClr(
		runtimePath.c_str(),
		"Nova-Host",
		propertyKeys.size(),
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
		void(*init)(void) = GetFunctionPtr<void(*)(void)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "init");
		updateScripts = GetFunctionPtr<void(*)(void)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "update");
		addGameObjectScript = GetFunctionPtr<void(*)(int, const char*)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "addGameObjectScript");
		removeGameObjectScript = GetFunctionPtr<void(*)(int, const char*)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "removeGameObjectScript");
		init();
		// Test Script 
		addGameObjectScript(0, "TestScript");
	}
	catch (...) {
		throw;
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

void ScriptingAPIManager::update() {
	updateScripts();
}
void ScriptingAPIManager::stopScriptingAPI()
{
	// Exit Test 
	removeGameObjectScript(0, "TestScript");
	// Shut down CoreClr
	int result{ shutdownCorePtr(hostHandle,domainID) };
	if (result != S_OK) {
		std::ostringstream errorDetails;
		errorDetails << "(0x";
		errorDetails << std::hex << result;
		errorDetails << ")Failed to shut down CoreCLR";
		throw std::runtime_error(errorDetails.str());
	}
}
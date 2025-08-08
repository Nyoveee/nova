#include <iostream>
#include <sstream>
#include <shlwapi.h>
#include <array>

#include "../Header/engine.h"
#include "../../window.h"
#include "../../Graphics/renderer.h"
#include "../../Graphics/cameraSystem.h"

#pragma comment(lib, "shlwapi.lib")

Engine::Engine()
	: coreClr{nullptr}
	, hostHandle{nullptr}
	, domainID{}
	, intializeCoreClr{nullptr}
	, createManagedDelegate{nullptr}
	, shutdownCorePtr(nullptr)
{
}
void Engine::initializeScriptingAPI()
{
	// Get the file path of the output directory containing coreclr.dll
	std::string runtimePath{std::string(MAX_PATH, '\0')};
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
	catch (...) {
		throw;
	}

	// Construct AppDomain Properties used when starting the runtime
	std::string tpaList{ buildTPAList(runtimePath) };
	std::array propertyKeys{"TRUSTED_PLATFORM_ASSEMBLIES", "APP_PATHS"};
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

}



std::string Engine::buildTPAList(const std::string& directory)
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

void Engine::stopScriptingAPI()
{
	int result{ shutdownCorePtr(hostHandle,domainID) };
	if (result != S_OK) {
		std::ostringstream errorDetails;
		errorDetails << "(0x";
		errorDetails << std::hex << result;
		errorDetails << ")Failed to shut down CoreCLR";
		throw std::runtime_error(errorDetails.str());
	}
}

void Engine::run() {
	Window&			window			= Window::instance();
	Renderer&		renderer		= Renderer::instance();
	CameraSystem&	cameraSystem	= CameraSystem::instance();
	try{
		initializeScriptingAPI();
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}
	

	window.run(
		// fixed update.
		[&](float fixedDt) {
			(void) fixedDt;
		},

		// normal update.
		[&](float dt) {
			cameraSystem.update();
			renderer.update(dt);
			renderer.render();
		}
	);
	try {
		void(*funcTest)(void) = GetFunctionPtr<void(*)(void)>("Nova-ScriptingAPI", "ScriptingAPI.Interface", "TestPrint");
		funcTest();
		stopScriptingAPI();
	}
	catch (std::exception e) {
		std::cout << e.what();
		return;
	}
	


}

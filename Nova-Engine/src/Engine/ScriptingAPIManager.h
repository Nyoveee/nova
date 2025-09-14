/*
	Reference:
	https://kahwei.dev/tag/scripting/
*/

#pragma once
#include "export.h"
#include "type_alias.h"

#include <entt/entt.hpp>
#include <dotnet/coreclrhost.h>

#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>

// Field Information
#ifndef ALL_FIELD_PRIMITIVES
#define ALL_FIELD_PRIMITIVES \
		bool, int, float, double, const char*
#endif
#ifndef ALL_FIELD_TYPES
	#define ALL_FIELD_TYPES \
		glm::vec2, glm::vec3, entt::entity, \
		ALL_FIELD_PRIMITIVES
#endif

using FieldData = std::pair<std::string, std::variant<ALL_FIELD_TYPES>>;

// More readable function pointer syntax because C lmaoo
using UpdateFunctionPtr				= void (*)(void);
using AddScriptFunctionPtr			= void (*)(unsigned int, std::size_t);
using RemoveScriptFunctionPtr		= void (*)(unsigned int, std::size_t);
using LoadScriptsFunctionPtr		= void (*)(void);
using UnloadScriptsFunctionPtr		= void (*)(void);
using IntializeScriptsFunctionPtr	= void (*)(void);
using ClearAllScriptsFunctionPtr    = void (*)(void);
using GetScriptFieldFunctionPtr     = std::vector<FieldData>(*)(unsigned int, unsigned long long);

class Engine;

class ScriptingAPIManager {
public:
	enum class CompileState {
		NotCompiled,
		ToBeCompiled,
		Compiled
	};

public:
	// Functions needed to run the ScriptingAPI
	DLL_API ScriptingAPIManager(Engine& p_engine);

	DLL_API ~ScriptingAPIManager();
	DLL_API ScriptingAPIManager(ScriptingAPIManager const& other)				= delete;
	DLL_API ScriptingAPIManager(ScriptingAPIManager&& other)					= delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager const& other)	= delete;
	DLL_API ScriptingAPIManager& operator=(ScriptingAPIManager&& other)			= delete;

public:
	DLL_API bool compileScriptAssembly();
	DLL_API void loadEntityScriptsFromScene();

	// Editor function
	DLL_API void loadEntityScript(unsigned int entityID, unsigned long long scriptID);
	DLL_API void removeEntityScript(unsigned int entityID, unsigned long long scriptID);

	// Simulation
	DLL_API bool startSimulation();
	DLL_API void stopSimulation();

	// Update
	DLL_API void update();
	DLL_API void checkModifiedScripts(float dt);

	// Serializable Field Reference
	DLL_API std::vector<FieldData> getScriptFieldDatas(unsigned int entityID, unsigned long long scriptID);


	// This is the callback when the assets files are Added
	DLL_API void OnAssetContentAddedCallback(std::string abspath);

	// This is the callback when the assets files are Modified
	DLL_API void OnAssetContentModifiedCallback(ResourceID assetTypeID);

	// This is the callback when the assets files are deleted
	DLL_API void OnAssetContentDeletedCallback(ResourceID assetTypeID);

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
	UpdateFunctionPtr updateScripts;
	AddScriptFunctionPtr addGameObjectScript;
	RemoveScriptFunctionPtr removeGameObjectScript;
	LoadScriptsFunctionPtr loadAssembly;
	UnloadScriptsFunctionPtr unloadAssembly;
	IntializeScriptsFunctionPtr initalizeScripts;
	GetScriptFieldFunctionPtr getScriptFieldDatas_;
	ClearAllScriptsFunctionPtr clearAllScripts_;
private:
	CompileState compileState;
	float timeSinceSave;

};

#include "Engine/ScriptingAPIManager.ipp"
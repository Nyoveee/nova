#include "ScriptingAPI.hxx"
#include "ScriptLibrary/Script.hxx"
#include "engine.h"
#include "assetManager.h"

#include <sstream>
#include <filesystem>

#include "ScriptLibraryHandler.hxx"

generic<typename T> where T : Script
T Interface::tryGetScriptReference(System::UInt32 entityID)
{
	// Go through the managed scripts
	for each (Script^ script in gameObjectScripts[entityID])
		if (script->GetType() == T::typeid)
			return safe_cast<T>(script); // Casting from one reference type to another
	return T(); // return null
}

void Interface::init(Engine& p_engine, const char* p_runtimePath)
{
	// Get the reference to the engine
	engine = &p_engine;
	runtimePath = p_runtimePath;
	gameObjectScripts = nullptr;
	scriptTypes = nullptr;
	ScriptLibraryHandler::init();
}

void Interface::addGameObjectScript(System::UInt32 entityID, ScriptID scriptId)
{
	if (!scriptTypes->ContainsKey(scriptId)) {
		Logger::error("Failed to add script {} for entity {}!", scriptId, entityID);
		return;
	}

	System::Type^ scriptType = scriptTypes[scriptId];

	if (!gameObjectScripts->ContainsKey(entityID)) {
		Scripts^ list = gcnew Scripts();
		gameObjectScripts->Add(entityID, list);
	}

	Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(scriptType));
	newScript->setEntityID(entityID);
	gameObjectScripts[entityID]->Add(newScript);
}

void Interface::removeGameObjectScript(System::UInt32 entityID, ScriptID scriptId)
{
	if (!scriptTypes->ContainsKey(scriptId)) {
		Logger::error("Failed to remove script {} for entity {}!", scriptId, entityID);
		return;
	}

	System::Type^ scriptType = scriptTypes[scriptId];

	for each (Script ^ script in gameObjectScripts[entityID]) {
		if (script->GetType() == scriptType) {
			script->callExit();
			gameObjectScripts[entityID]->Remove(script);
			return;
		}
	}
}

void Interface::intializeAllScripts()
{
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (Script ^ script in gameObjectScripts[entityID])
			script->callInit();
}

void Interface::update() {
	ScriptLibraryHandler::update();
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (Script ^ script in gameObjectScripts[entityID])
			script->callUpdate();
}

void Interface::load()
{
	// ========================================================
	// 1. Load C# .dll to assembly context.
	// ========================================================
	assemblyLoadContext = gcnew System::Runtime::Loader::AssemblyLoadContext(nullptr, true);
	
	std::filesystem::path originalPath{ std::filesystem::current_path() };
	const char* path{ runtimePath };
	std::filesystem::current_path(path); // Set to the output directory to load nova-scripts.dll from
	System::IO::FileStream^ scriptLibFile = System::IO::File::Open(
		"Nova-Scripts.dll",
		System::IO::FileMode::Open, System::IO::FileAccess::Read
	);
	std::filesystem::current_path(originalPath); // Reset the path	
	
	assemblyLoadContext->LoadFromStream(scriptLibFile);
	scriptLibFile->Close();

	// ========================================================
	// 2. Instantiate the respective containers.
	// ========================================================
	gameObjectScripts = gcnew System::Collections::Generic::Dictionary<System::UInt32, System::Collections::Generic::List<Script^>^>();
	scriptTypes = gcnew System::Collections::Generic::Dictionary<ScriptID, System::Type^>();

	// ========================================================
	// 3. We first find our loaded nova script assembly.
	// ========================================================
	System::Reflection::Assembly^ novaScriptAssembly = nullptr;

	for each (System::Reflection::Assembly ^ assembly in assemblyLoadContext->Assemblies) {
		if (assembly->GetName()->Name == "Nova-Scripts") {
			novaScriptAssembly = assembly;
			break;
		}
	}

	if (!novaScriptAssembly) {
		Logger::error("Failed to find loaded nova script assembly!");
		return;
	}

	// ========================================================
	// 4. Load all script types in loaded assembly.
	// ========================================================
	
	// We maintain a temporary dictionary mapping class names to script types.
	System::Collections::Generic::Dictionary<System::String^, System::Type^> classNameToScriptType;
	for each (System::Type ^ type in novaScriptAssembly->GetTypes()) {
		if (!type->IsSubclassOf(Script::typeid))
			continue;
		classNameToScriptType.Add(type->Name, type);
	}
	
	// ========================================================
	// 5. We define mapping from Asset IDs to script types.
	// ========================================================
	auto&& scripts = engine->assetManager.getAllAssets<ScriptAsset>();
	for (auto&& scriptId : scripts) {
		auto&& [script, _] = engine->assetManager.getAsset<ScriptAsset>(scriptId);
		assert(script && "Script should always be instantly available.");

		System::String^ className = gcnew System::String(script->getClassName().c_str());
		if (!classNameToScriptType.ContainsKey(className)) {
			Logger::warn("Script asset {} contains invalid class name. Filename and class name probably don't match."
						 "\nThis script is not loaded.", script->getFilePath());
			continue;
		}

		System::Type^ scriptType = classNameToScriptType[className];
		scriptTypes->Add(static_cast<std::size_t>(scriptId), scriptType);
	}
}

void Interface::unload()
{
	// Clear existing scripts
	if(gameObjectScripts)	gameObjectScripts->Clear();
	if(scriptTypes)			scriptTypes->Clear();

	if (assemblyLoadContext) {
		// Unload the assembly
		assemblyLoadContext->Unload();
		assemblyLoadContext = nullptr;
	}

	// Garbage Collect existing memory
	System::GC::Collect();
	// Wait from assembly to finish unloading
	System::GC::WaitForPendingFinalizers();
}

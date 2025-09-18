#include "ScriptingAPI.hxx"
#include "ScriptLibrary/Script.hxx"
#include "ScriptLibraryHandler.hxx"
#include "IManagedComponent.hxx"
#include "IManagedStruct.h"
#include "ResourceManager/resourceManager.h"

#include <sstream>
#include <filesystem>
#include <msclr/marshal_cppstd.h>



generic<typename T> where T : Script
T Interface::tryGetScriptReference(System::UInt32 entityID)
{
	// Go through the managed scripts
	for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys)
		if (gameObjectScripts[entityID][scriptID]->GetType() == T::typeid)
			return safe_cast<T>(gameObjectScripts[entityID][scriptID]); // Casting from one reference type to another
	return T(); // return null
}

void Interface::init(Engine& p_engine, const char* p_runtimePath)
{
	// Get the reference to the engine
	engine = &p_engine;
	runtimePath = p_runtimePath;
	ScriptLibraryHandler::init();
	// Instantiate the containers
	gameObjectScripts = gcnew System::Collections::Generic::Dictionary<System::UInt32, System::Collections::Generic::Dictionary<System::UInt64,Script^>^>();
	scriptTypes = gcnew System::Collections::Generic::Dictionary<ScriptID, System::Type^>();
}

void Interface::intializeAllScripts()
{
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys)
			gameObjectScripts[entityID][scriptID]->callInit();
}

std::vector<FieldData> Interface::getScriptFieldDatas(EntityID entityID, ScriptID scriptID)
{
	using BindingFlags = System::Reflection::BindingFlags;
	std::vector<FieldData> fieldDatas{};
	Script^ script = gameObjectScripts[entityID][scriptID];
	array<System::Reflection::FieldInfo^>^ fieldInfos = script->GetType()->GetFields(BindingFlags::Instance | BindingFlags::Public | BindingFlags::NonPublic);
	for (int i = 0; i < fieldInfos->Length; ++i) {
		// Ignore the base class
		if (fieldInfos[i]->DeclaringType == Script::typeid)
			continue;
		System::Type^ fieldType = fieldInfos[i]->GetModifiedFieldType()->UnderlyingSystemType;
		FieldData field{};
		// Private and Protected members will only be added if they have the serializablefield attribute
		if (!fieldInfos[i]->IsPublic && fieldInfos[i]->GetCustomAttributes(SerializableField::typeid,true)->Length == 0)
			continue;
		field.first = msclr::interop::marshal_as<std::string>(fieldInfos[i]->Name);
		// Struct
		if (IManagedStruct^ managedStruct = dynamic_cast<IManagedStruct^>(fieldInfos[i]->GetValue(script))) {
			managedStruct->AppendValueToFieldData(field);
			fieldDatas.push_back(field);
			continue;
		}
		// Component
		if (fieldType->IsSubclassOf(IManagedComponent::typeid)) {
			IManagedComponent^ managedComponent = safe_cast<IManagedComponent^>(fieldInfos[i]->GetValue(script));
			field.second = entt::entity(managedComponent ? managedComponent->entityID : entt::null);
			fieldDatas.push_back(field);
			continue;
		}
		// Scripts
		if (fieldType->IsSubclassOf(Script::typeid)) {
			Script^ managedScripts = safe_cast<Script^>(fieldInfos[i]->GetValue(script));
			field.second = entt::entity(managedScripts ? managedScripts->entityID : entt::null);
			fieldDatas.push_back(field);
			continue;
		}
		// Primitives
		if (fieldType->IsPrimitive && ObtainPrimitiveDataFromScript<ALL_FIELD_PRIMITIVES>(field, fieldInfos[i]->GetValue(script))) {
			fieldDatas.push_back(field);
			continue;
		}
		if (fieldType->IsPrimitive) {
			Logger::warn("Primitive type in script currently not supported for script serialization {}",
				msclr::interop::marshal_as<std::string>(fieldType->ToString()));
		}
	}
	return fieldDatas;
}

void Interface::setScriptFieldData(EntityID entityID, ScriptID scriptID, FieldData const& fieldData)
{
	Script^ script = gameObjectScripts[entityID][scriptID];;
	using BindingFlags = System::Reflection::BindingFlags;
	array<System::Reflection::FieldInfo^>^ fieldInfos = script->GetType()->GetFields(BindingFlags::Instance | BindingFlags::Public | BindingFlags::NonPublic);
	for (int i = 0; i < fieldInfos->Length; ++i) {
		// Ignore private and protected members if it doesn't have the serializablefield attribute
		if (!fieldInfos[i]->IsPublic && fieldInfos[i]->GetCustomAttributes(SerializableField::typeid, true)->Length == 0)
			continue;
		// Field names are always unique
		if (msclr::interop::marshal_as<std::string>(fieldInfos[i]->Name) != fieldData.first)
			continue;
		System::Type^ fieldType = fieldInfos[i]->GetModifiedFieldType()->UnderlyingSystemType;
		// Struct
		if (IManagedStruct^ managedStruct = dynamic_cast<IManagedStruct^>(fieldInfos[i]->GetValue(script))) {
			// Set the value of the copy
			managedStruct->SetValueFromFieldData(fieldData);
			fieldInfos[i]->SetValue(script, managedStruct);
			return;
		}
		// Component
		if (fieldType->IsSubclassOf(IManagedComponent::typeid)) {
			IManagedComponent^ managedComponent = safe_cast<IManagedComponent^>(fieldInfos[i]->GetValue(script));
			managedComponent->entityID = static_cast<unsigned int>(std::get<entt::entity>(fieldData.second));
			return;
		}
		// Script
		if (fieldType->IsSubclassOf(Script::typeid)) {
			script->entityID = static_cast<unsigned int>(std::get<entt::entity>(fieldData.second));
			return;
		}
		// Primitives
		if (SetScriptPrimitiveFromNativeData<ALL_FIELD_PRIMITIVES>(fieldData, script, fieldInfos[i]))
			return;
		if (fieldType->IsPrimitive)
			Logger::warn("Unknown primitive type used in setting fields currently not supported for script serialization");
	}
}

void Interface::update() {
	ScriptLibraryHandler::update();
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys)
			gameObjectScripts[entityID][scriptID]->callUpdate();
}

void Interface::addGameObjectScript(EntityID entityID, ScriptID scriptId)
{
	if (!scriptTypes->ContainsKey(scriptId)) {
		Logger::error("Failed to add script {} for entity {}!", scriptId, entityID);
		return;
	}

	System::Type^ scriptType = scriptTypes[scriptId];
	if (!gameObjectScripts->ContainsKey(entityID))
		gameObjectScripts[entityID] = gcnew Scripts();

	Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(scriptType));
	newScript->entityID = entityID;
	gameObjectScripts[entityID][scriptId] = newScript;
}

void Interface::removeGameObjectScript(EntityID entityID, ScriptID scriptId)
{
	if (!scriptTypes->ContainsKey(scriptId)) {
		Logger::error("Failed to remove script {} for entity {}!", scriptId, entityID);
		return;
	}
	gameObjectScripts[entityID]->Remove(scriptId);
}

void Interface::clearAllScripts()
{
	if (gameObjectScripts)	gameObjectScripts->Clear();
}

void Interface::loadAssembly()
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
	// 2. We first find our loaded nova script assembly.
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
	// 3. Load all script types in loaded assembly.
	// ========================================================
	
	// We maintain a temporary dictionary mapping class names to script types.
	System::Collections::Generic::Dictionary<System::String^, System::Type^> classNameToScriptType;
	for each (System::Type ^ type in novaScriptAssembly->GetTypes()) {
		if (!type->IsSubclassOf(Script::typeid))
			continue;
		classNameToScriptType.Add(type->Name, type);
	}
	
	// ========================================================
	// 4. We define mapping from Resource IDs to script types.
	// ========================================================
	auto&& scripts = engine->resourceManager.getAllResources<ScriptAsset>();
	for (auto&& scriptId : scripts) {
		auto&& [script, _] = engine->resourceManager.getResource<ScriptAsset>(scriptId);
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

void Interface::unloadAssembly()
{
	// Clear existing scripts
	clearAllScripts();
	if (scriptTypes)
		scriptTypes->Clear();
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

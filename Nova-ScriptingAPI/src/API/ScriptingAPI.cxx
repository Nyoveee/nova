// Converting to native type: https://learn.microsoft.com/en-us/cpp/dotnet/overview-of-marshaling-in-cpp?view=msvc-170
#include "ScriptingAPI.hxx"
#include "ScriptLibrary/Script.hxx"
#include "ecs.h"

#include <sstream>
#include <filesystem>
#include <msclr/marshal_cppstd.h>
generic<typename T> where T : Script
T Interface::tryGetScriptReference(System::UInt32 entityID)
{
	// Go through the managed scripts
	for each (Script^ script in gameObjectScripts[entityID])
		if (script->GetType() == T::typeid)
			return safe_cast<T>(script); // Casting from one reference type to another
	return T(); // <---- WHY IS THIS CONSIDERED A NULLPTR?
}
void Interface::init(ECS& ecs, const char* runtimePath)
{
	registry = &ecs.registry;
	gameObjectScripts = gcnew System::Collections::Generic::Dictionary<System::UInt32, System::Collections::Generic::List<Script^>^>();
	// Load the dll for calling the functions
	std::filesystem::path originalPath{ std::filesystem::current_path() };
	std::filesystem::current_path(runtimePath); // Set to the output directory to load nova-scripts.dll from
	System::Reflection::Assembly::LoadFrom("Nova-Scripts.dll");
	std::filesystem::current_path(originalPath); // Reset the path	

	// Load all the c# scripts to run
	for each (System::Reflection::Assembly ^ assembly in System::AppDomain::CurrentDomain->GetAssemblies()) {
		if (assembly->GetName()->Name != "Nova-Scripts")
			continue;
		for each (System::Type^ type in assembly->GetTypes()) {
			if (!type->IsSubclassOf(Script::typeid))
				continue;
			scriptTypes.Add(type);
		}
	}
}

void Interface::addGameObjectScript(System::UInt32 entityID, System::String^ scriptName)
{
	for each (System::Type ^ type in scriptTypes) {
		if (type->Name == scriptName) {
			if (!gameObjectScripts->ContainsKey(entityID)) {
				Scripts^ list = gcnew Scripts();
				gameObjectScripts->Add(entityID, list);
			}
			Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(type));
			newScript->setEntityID(entityID);
			newScript->callInit();

			gameObjectScripts[entityID]->Add(newScript);
			return;
		}
	}
	std::ostringstream errorDetails;
	errorDetails << "(ID = " << std::to_string(static_cast<entt::id_type>(entityID)) << ") Script not found for adding";
	throw std::runtime_error(errorDetails.str());
}
void Interface::removeGameObjectScript(System::UInt32 entityID, System::String^ scriptName)
{
	for each (Script ^ script in gameObjectScripts[entityID]) {
		if (script->GetType()->Name == scriptName) {
			script->callExit();
			gameObjectScripts[entityID]->Remove(script);
			return;
		}
	}
	std::ostringstream errorDetails;
	errorDetails << "(ID = " << std::to_string(static_cast<entt::id_type>(entityID)) << ") Script not found for removal";
	throw std::runtime_error(errorDetails.str());
}

std::vector<std::string> Interface::getScriptNames()
{
	using namespace msclr::interop;
	std::vector<std::string> scriptNames{};
	for each (System::Type ^ type in scriptTypes)
		scriptNames.push_back(marshal_as<std::string, System::String^>(type->Name));
	return scriptNames;
}

void Interface::update() {
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (Script ^ script in gameObjectScripts[entityID])
			script->callUpdate();
}
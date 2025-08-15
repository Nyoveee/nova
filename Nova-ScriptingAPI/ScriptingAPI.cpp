// Converting to native type: https://learn.microsoft.com/en-us/cpp/dotnet/overview-of-marshaling-in-cpp?view=msvc-170
#include <sstream>
#include <filesystem>
#include "ScriptingAPI.h"
#include <msclr/marshal_cppstd.h>
// Some Weird intellisense error is happening in this file, but no build error 
using namespace msclr::interop;
namespace ScriptingAPI {
	void Interface::init(Engine& newEngine, const char* runtimePath)
	{
		engine = &newEngine;
		// Intialize the dictionary
		gameObjectScripts = gcnew System::Collections::Generic::Dictionary <System::UInt32, Scripts^>();
		// Load the dll for calling the functions
		std::filesystem::path originalPath{ std::filesystem::current_path() };
		std::filesystem::current_path(runtimePath); // Set to the output directory to load nova-scripts.dll from
		System::Reflection::Assembly::LoadFrom("Nova-Scripts.dll");
		std::filesystem::current_path(originalPath); // Reset the path	
		// Load all the c# scripts to run
		for each (System::Reflection::Assembly ^ assembly in System::AppDomain::CurrentDomain->GetAssemblies()) {
			if (assembly->GetName()->Name != "Nova-Scripts")
				continue;
			for each (System::Type ^ type in assembly->GetTypes()) {
				if (!type->IsSubclassOf(Script::typeid))
					continue;
				scriptTypes.Add(type);
			}
		}
	}
	void Interface::addGameObjectScript(System::UInt32 entityID, System::String^ scriptName)
	{
		for each (System::Type^ type in scriptTypes) {
			if (type->Name == scriptName) {
				Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(type));
				newScript->LoadEntityID(entityID);
				newScript->Init();
				Scripts^ list = gcnew Scripts();
				if (!gameObjectScripts->ContainsKey(entityID))
					gameObjectScripts->Add(entityID,list);
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
		for each (Script^ script in gameObjectScripts[entityID]) {
			if (script->GetType()->Name == scriptName) {
				script->exit();
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
		std::vector<std::string> scriptNames{};
		for each (System::Type^ type in scriptTypes)
			scriptNames.push_back(marshal_as<std::string,System::String^>(type->Name));
		return scriptNames;
	}

	void ScriptingAPI::Interface::update(){
		for each(System::UInt32 entityID in gameObjectScripts->Keys)
			for each(Script^ script in gameObjectScripts[entityID])
				script->update();
	}
}
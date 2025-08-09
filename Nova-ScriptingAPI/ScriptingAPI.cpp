#include <sstream>

#include "ScriptingAPI.h"
namespace ScriptingAPI {
	void Interface::init()
	{
		// Intialize the dictionary
		gameObjectScripts = gcnew System::Collections::Generic::Dictionary<int, Scripts^>();
	
		// Load the dll for calling the functions
		System::Reflection::Assembly::LoadFrom("Nova-Scripts.dll");
		// Load all the c# scripts to run
		for each(System::Reflection::Assembly^ assembly in System::AppDomain::CurrentDomain->GetAssemblies()) {
			if (assembly->GetName()->Name != "Nova-Scripts")
				continue;
			for each (System::Type ^ type in assembly->GetTypes()) {
				if (!type->IsSubclassOf(Script::typeid))
					continue;
				scriptTypes.Add(type);
			}
		}
	}
	void Interface::addGameObjectScript(int gameObjectID, System::String^ scriptName)
	{
		for each (System::Type^ type in scriptTypes) {
			if (type->Name == scriptName) {
				Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(type));
				newScript->Init();
				Scripts^ list = gcnew Scripts();
				if (!gameObjectScripts->ContainsKey(gameObjectID))
					gameObjectScripts->Add(gameObjectID,list);
				gameObjectScripts[gameObjectID]->Add(newScript);
				return;
			}
		}
		std::ostringstream errorDetails;
		errorDetails << "(ID = " << std::to_string(gameObjectID) << ") Script not found for adding";
		throw std::runtime_error(errorDetails.str());
	}
	void Interface::removeGameObjectScript(int gameObjectID, System::String^ scriptName)
	{
		for each (Script ^ script in gameObjectScripts[gameObjectID]) {
			if (script->GetType()->Name == scriptName) {
				script->Exit();
				gameObjectScripts[gameObjectID]->Remove(script);
				return;
			}
		}
		std::ostringstream errorDetails;
		errorDetails << "(ID = " << std::to_string(gameObjectID) << ") Script not found for removal";
		throw std::runtime_error(errorDetails.str());
	}

	void ScriptingAPI::Interface::update(){
		for each(int gameobjectID in gameObjectScripts->Keys)
			for each(Script^ script in gameObjectScripts[gameobjectID])
				script->update();
	}
}


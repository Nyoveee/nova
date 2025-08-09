// Manages the c# scripts for each gameobject

#pragma once
#include "Script.h"

namespace ScriptingAPI {
	public ref class Interface
	{
	public:
		static void init();
		static void update();
		// GameObjectID should be unique and created somewhere else
		static void addGameObjectScript(int gameObjectID, System::String^ scriptName);
		static void removeGameObjectScript(int gameObjectID, System::String^ scriptName);
	
	private:
		using Scripts = System::Collections::Generic::List<Script^>;
		static System::Collections::Generic::Dictionary<int, Scripts^>^ gameObjectScripts;
		static System::Collections::Generic::Dictionary<int, bool>^ isGameObjectActive; // Will do once gameobject implementation is there
		static System::Collections::Generic::List<System::Type^> scriptTypes;
	};
}



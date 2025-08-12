// Base C# Script
// https://learn.microsoft.com/en-us/cpp/dotnet/cpp-cli-tasks?view=msvc-170
//https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html
#pragma once
#include "ManagedTypes.h"
namespace ScriptingAPI {
	public ref class Script abstract
	{
	internal:
		void LoadEntityID(System::UInt32 entityID) { this->entityID = entityID; }
	public:
		virtual void Init() {};
		virtual void update() {};
		virtual void exit() {};	
		generic<typename Component> where Component: IManagedComponent 
		Component getComponent() { 
			// Can't use constructor cause it assumes it's a function for some reason
			Component component;
			component->SetEntityID(entityID);
			return component; 
		};
	private:
		System::UInt32 entityID;
	};
	
}




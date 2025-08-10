// Base C# Script
// https://learn.microsoft.com/en-us/cpp/dotnet/cpp-cli-tasks?view=msvc-170
//https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html
#pragma once
#include "ManagedComponents.h"
#include "../Nova-Engine/src/Export/Header/ECS.h"
namespace ScriptingAPI {
	public ref class Script abstract
	{
	public:
		virtual void Init() {};
		virtual void update() {};
		virtual void exit() {};	
		Transform_ GetTransformComponent();
		void SetEntityID(System::UInt32 newEntityID); // Make this not overridable by the inherited scripts
	private:
		System::UInt32 entityID;
	};
	
}




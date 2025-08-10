// Create the managed class versions that links with the native class versions for the scripts to use
// Must build the nova-scriptingAPI project for intellisense to show updated components for the scripts
#pragma once
#include "../Nova-Engine/src/Export/Header/component.h"
#include "../Nova-Engine/src/Export/Header/ECS.h"
// To do, Macro to generate the managed class version of the components in component.h
// Call Entt to get the component
namespace ScriptingAPI {
	// Managed type
	public value struct Vector3 {
		float x;
		float y;
		float z;
	};
	public value struct Transform_ {
	public:
		Transform_(System::UInt32 entityID);
		property Vector3 position
		{
			Vector3 get();
			void set(Vector3 value);
		}
	private:
		System::UInt32 entityID;
	};
}
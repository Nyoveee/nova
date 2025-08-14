/*
Create the managed types that reference the native types(The Actual data being used) for the scripts to use
Usage of macros:
	Both types support up to 16 variables
	ManagedStruct: 
		Creates a Managed version of a struct in native code
		For Example,
		ManagedStruct(Vector3, glm::vec3, float, x, float, y, float, z)
		Vector3 is the struct used by the scripts
		glm::vec3 is the native code version for the API to modify data
		Then choose the public variables matching the native code's struct that 
		you want the scripts to use such as float x
	ManagedComponentDeclaration:
		Similiar to ManagedStruct, These can use getComponent Function()
		Define the managed component and then declare the variables to use matching the names in component.h
		After that go to ManagedComponents.cpp to set up the definition macros
*/
#pragma once
#include "../Nova-Engine/src/Component/component.h"
#include "ManagedTypeMacros.h"
namespace ScriptingAPI {
	ManagedStruct(Vector3, glm::vec3, float, x, float, y, float, z)

	ManagedComponentDeclaration(Transform_, Vector3, position, Vector3, rotation, Vector3, scale)
}
/*
Create the managed types that reference the native types(The Actual data being used) for the scripts to use
Usage of macros:
	ManagedComponentDefinition:
		This will define the getters and setters
		Example
		ManagedComponentDefinition(Transform_, Transform, Vector3, position)
		Transform_ is the class to define the getter and setter
		Transform is the native component to get the data from the entity
		Then the type and name for the code to generate the functions
*/
#include "ManagedTypes.h"
#include "ScriptingAPI.h"
// Non struct types like float and int is not implemented yet
ManagedComponentDefinition(Transform_, Transform, Vector3, position)
ManagedComponentDefinition(Transform_, Transform, Vector3, rotation)
ManagedComponentDefinition(Transform_, Transform, Vector3, scale)




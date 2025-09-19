#pragma once

#include "ECS/component.h"
#include "API/IManagedComponent.hxx"
#include "API/ManagedTypeMacros.hxx"

// ===========================================================================================
// 1. Defining structs..
// 
// This creates a new managed type, that will be associated with the given native type. (must be an aggregate class!!)
// This is important as C# Scripts can only interact with managed types and not native types directly.
// Provide the respective data members you want your managed type to contain. Name of the data member must match
// exactly with the native type.
// 
// Providing association here also allows you to declare data members of Managed Components
// later in 2. with the managed type.
// ===========================================================================================
ManagedStruct(
	Vector2, glm::vec2,
	float, x,
	float, y
)
ManagedStruct(
	Vector3, glm::vec3,		// Creates a new managed type Vector3 that is associated with glm::vec3
	float, x,
	float, y,
	float, z
)
// New managed type Vector3 now has data member of x, y, z corresponding to the data members of glm::vec3.

// ===========================================================================================
// 2. Defining managed component types..
// 
// This defines a new managed component, that will be associated with the given native component. 
// The new name of the managed component would append a underscore `XXXX_` at the back.
// 
// List down the respective data members you want the managed component to have access, along with
// the type. The name of the data member must match exactly with the identifier in the Component!
// 
// The type of the data member must match with the original data member. It must either be a primitive type, 
// or a managed type associated with the native type of the data member.
// 
// Defining a component here allows the C# script to call GetComponent<...> to retrieve the components
// of a given entity.
// ===========================================================================================
ManagedComponentDeclaration(
	Transform,					// Creates a new managed component Transform_ that is associated with the Transform component
	Vector3, position,			// Transform_ now has data member position, of type Vector3 which is associated with glm::vec3 (type of original data member).
	Vector3, scale
)
// We created a Managed Component named Transform_, with data members position, scale, rotation, test1 and test2.

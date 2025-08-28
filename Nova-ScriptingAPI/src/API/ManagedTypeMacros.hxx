#pragma once

#include "RecursiveMacros.hxx"
#include "ScriptingAPI.hxx"
#include <type_traits>


// This default function template returns the managed type as it is
// This is used for primitives like ints, floats, etc, where you can return the managed type as native type directly.
// Managed structs will definite an explicit specialisation of this function template

// If you received error in regards to narrowing conversion, there's a high chance you provided the wrong types!
template <typename ManagedType, typename NativeType>
NativeType native(ManagedType& managedType) {
	return NativeType{ managedType };
}

// =====================================================
// For managed structs -> to convert native types to managed types for interaction
// =====================================================
#define Declaration(Type, Name) Type Name;
#define ConstructorDefinition(Type, Name) Name{native.Name},
#define ListInitialization(Type, Name) Name,

#define ManagedStruct(ManagedType,NativeType,...)																	\
public value struct ManagedType {																				    \
	ManagedType(NativeType native) : Call_Macro_Double(ConstructorDefinition, __VA_ARGS__) type { #ManagedType } {} \
	NativeType native() { return {Call_Macro_Double(ListInitialization, __VA_ARGS__)};}								\
	Call_Macro_Double(Declaration,__VA_ARGS__)																		\
	System::String^ type;																							\
};																													\
																													\
template <>																											\
NativeType native(ManagedType& managedType) {																		\
	return managedType.native();																					\
}	

// =====================================================
// Managed component macro generator.
// =====================================================
#define PropertyDeclaration(ComponentType, Type, Name)																					\
property Type Name																														\
{																																		\
	Type get()				{ return Type(Interface::getNativeComponent<ComponentType>(entityID)->Name); }				                \
	void set(Type value)	{																											\
		using NativeType = decltype(Interface::getNativeComponent<ComponentType>(entityID)->Name);							            \
		Interface::getNativeComponent<ComponentType>(entityID)->Name = native<Type, NativeType>(value);	                                \
	}																																	\
}

#define ManagedComponentDeclaration(ComponentType, ...) \
public ref class ComponentType##_ : IManagedComponent { \
public: \
	Call_Macro_Double_C(PropertyDeclaration, ComponentType, __VA_ARGS__) \
internal: \
virtual bool exist(System::UInt32 p_entityID) override sealed { return Interface::getNativeComponent<ComponentType>(p_entityID) != nullptr; } \
private: \
	System::UInt32 entityID; \
};
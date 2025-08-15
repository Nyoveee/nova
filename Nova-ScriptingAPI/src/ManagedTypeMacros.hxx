#pragma once

#include "RecursiveMacros.hxx"
#include <type_traits>

// =====================================================
// For managed structs -> to convert native types to managed types for interaction
// =====================================================
#define Declaration(Type, Name) Type Name;
#define ConstructorDefinition(Type, Name) Name{native.Name},
#define ListInitialization(Type, Name) Name,

#define ManagedStruct(ManagedType,NativeType,...)																	\
public value struct ManagedType {																					\
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
	Type get()				{ return Type(ScriptingAPI::Interface::findNativeComponent<ComponentType>(entityID)->Name); }				\
	void set(Type value)	{																											\
		using NativeType = decltype(ScriptingAPI::Interface::findNativeComponent<Transform>(entityID)->Name);							\
		ScriptingAPI::Interface::findNativeComponent<ComponentType>(entityID)->Name = ScriptingAPI::native<Type, NativeType>(value);	\
	}																																	\
}

#define ManagedComponentDeclaration(ComponentType, ...) \
public value class ComponentType##_ : IManagedComponent { \
public: \
	virtual void SetEntityID(System::UInt32 p_entityID) { this->entityID = p_entityID; }; \
	Call_Macro_Double_C(PropertyDeclaration, ComponentType, __VA_ARGS__) \
private: \
	System::UInt32 entityID; \
};

#if 0
#define ManagedComponentDefinition(ManagedComponentType,NativeComponentType, ManagedStructType, Name) \
ScriptingAPI::ManagedStructType ScriptingAPI::ManagedComponentType::Name::get() \
{ \
	return ManagedStructType{ ScriptingAPI::Interface::findNativeComponent<NativeComponentType>(entityID)->Name }; \
} \
void ScriptingAPI::ManagedComponentType::Name::set(ManagedStructType value) \
{ \
	ScriptingAPI::Interface::findNativeComponent<NativeComponentType>(entityID)->Name = value.native(); \
} 
#endif
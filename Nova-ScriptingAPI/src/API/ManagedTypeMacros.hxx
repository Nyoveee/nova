#pragma once

#include "IManagedStruct.h"
#include "IManagedComponent.hxx"

#include "RecursiveMacros.hxx"
#include "ScriptingAPI.hxx"
#include "ScriptLibrary/Core/GameObject.hxx"
#include "Logger.h"

#include <type_traits>
#include <unordered_set>
// This default function template returns the managed type as it is
// This is used for primitives like ints, floats, etc, where you can return the managed type as native type directly.
// Managed structs will definite an explicit specialisation of this function template

template <typename Managed>
struct ManagedToNative {
	using Native = Managed;
};

// If you received error in regards to narrowing conversion, there's a high chance you provided the wrong types!
template <typename NativeType, typename ManagedType>
NativeType native(ManagedType managedType) {
	return NativeType{ managedType };
}

// =====================================================
// For managed structs -> to convert native types to managed types for interaction
// =====================================================
#define Declaration(Type, Name) Type Name;
#define ConstructorDefinition(Type, Name) Name{ native.Name }
#define ConstructorDefinition2(Type, Name) Name { Name }

#define ListInitialization(Type, Name) ::native<ManagedToNative<Type>::Native>(Name)

#define Parameter(Type, Name) Type Name

#define ManagedStruct(ManagedType,NativeType,...)																					\
public value struct ManagedType : IManagedStruct {																					\
	ManagedType(NativeType native) : Call_MacroComma_Double(ConstructorDefinition, __VA_ARGS__) {}									\
	ManagedType(Call_MacroComma_Double(Parameter,__VA_ARGS__)) : Call_MacroComma_Double(ConstructorDefinition2, __VA_ARGS__) {}     \
	NativeType native() { return {Call_MacroComma_Double(ListInitialization , __VA_ARGS__)};}										\
	virtual void AppendValueToFieldData(FieldData& fieldData) sealed{															    \
		fieldData.data = native();																									\
	}																																\
	virtual void SetValueFromFieldData(FieldData const& fieldData) sealed{                                                          \
		*this = ManagedType(std::get<NativeType>(fieldData.data));																	\
	}                                                                                                                               \
	virtual System::String^ ToString() override sealed{																				\
		array<System::Reflection::FieldInfo^>^ fieldInfos = GetType()->GetFields();													\
		System::String^ result = "{";																								\
		for (int i = 0; i < fieldInfos->Length; ++i) {																				\
			result += fieldInfos[i]->GetValue(*this)->ToString();																	\
			if (i != fieldInfos->Length - 1) result += ", ";																		\
		}																															\
		result += "}";																												\
		return result;																												\
	}																																\
	Call_Macro_Double(Declaration,__VA_ARGS__)																						\

#define ManagedStructEnd(ManagedType,NativeType)																					\
};																																	\
template <typename T>																												\
inline T native(ManagedType managedType){																							\
	return managedType.native();																									\
}																																	\
template <>																															\
struct ManagedToNative<ManagedType> {																								\
	using Native = NativeType;																										\
};																																	\
// =====================================================
// Managed component macro generator.
// =====================================================
#define PropertyDeclaration(Type, Name)														\
property Type Name																			\
{																							\
	Type get()				{																\
		return Type(static_cast<ManagedToNative<Type>::Native>(componentReference->Name));	\
	}																						\
	void set(Type value)	{																\
		using NativeType = decltype(componentReference->Name);								\
		componentReference->Name = native<ManagedToNative<Type>::Native>(value);			\
	}																						\
}

#define ManagedComponentDeclaration(ComponentType, ...)																			\
public ref class ComponentType##_ : IManagedComponent {																			\
public:																															\
	Call_Macro_Double(PropertyDeclaration, __VA_ARGS__)																			\
	virtual System::String^ ToString() override sealed{																			\
		array<System::Reflection::PropertyInfo^>^ propertyInfos = GetType()->GetProperties();									\
		System::String^ result = #ComponentType + "_{";																			\
		for (int i = 0; i < propertyInfos->Length; ++i) {																		\
			result += propertyInfos[i]->Name;																					\
			result += propertyInfos[i]->GetValue(this)->ToString();																\
			if (i != propertyInfos->Length - 1) result += ", ";																	\
		}																														\
		result += "}";																											\
		return result;																											\
	}																															\
internal:																														\
	ComponentType* nativeComponent() { return Interface::getNativeComponent<ComponentType>(entityID); }							\
	bool NativeReferenceLost() override { return !componentReference; }															\
	bool LoadDetailsFromEntity(System::UInt32 p_entityID) override {															\
		entityID = p_entityID;																									\
		if ((componentReference = Interface::getNativeComponent<ComponentType>(entityID)))										\
			return true;																										\
		return false;																											\
	}																															\
public:																															\
	property GameObject^ gameObject {																							\
		GameObject^ get() { return GameObject::GetReference(entityID); };														\
	}																															\
	property bool enable{																										\
		bool get(){																												\
			EntityData& entityData{ Interface::engine->ecs.registry.get<EntityData>(static_cast<entt::entity>(entityID)) };		\
			return NonComponentDisablingTypes<ComponentType>																	\
				|| !entityData.inactiveComponents.count(typeid(ComponentType).hash_code());										\
		};																														\
		void set(bool b_Enable){																								\
			if constexpr (NonComponentDisablingTypes<ComponentType>) {															\
				Logger::warn("{} does not support enabling/disabling", #ComponentType);											\
				return;																											\
			}																													\
			EntityData& entityData{ Interface::engine->ecs.registry.get<EntityData>(static_cast<entt::entity>(entityID)) };		\
			std::unordered_set<size_t>& inactiveComponents{ entityData.inactiveComponents };									\
			size_t componentID{ typeid(ComponentType).hash_code() };															\
			if (b_Enable && !get())																								\
				inactiveComponents.insert(componentID);																			\
			else if(!b_Enable && get())																							\
				inactiveComponents.erase(std::find(std::begin(inactiveComponents), std::end(inactiveComponents), componentID));	\
			return;																												\
		};																														\
	}																															\
private:																														\
	ComponentType* componentReference;																							\
public:																															\

#define ManagedComponentEnd()																		};

#define ManagedResource(Resource)																	\
public ref class Resource : IManagedResourceID {													\
public:																								\
	Resource(TypedResourceID<::Resource> id) : IManagedResourceID(static_cast<std::size_t>(id)) {};	\
																									\
	virtual System::String^ ToString() override sealed {											\
		return resourceID.ToString();																\
	}																								\
																									\
	TypedResourceID<::Resource> getId() {															\
		return { resourceID };																		\
	};																								\
};																									
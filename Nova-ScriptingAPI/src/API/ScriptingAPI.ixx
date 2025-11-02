#include "ScriptingAPI.hxx"
#include "Engine/engine.h"
#include "ScriptLibrary/Extensions/ManagedTypes.hxx"
#include <iostream>

template<typename T>
T* Interface::getNativeComponent(System::UInt32 entityID) {
	// Get the Entt reference from the engine
	entt::entity entity{ static_cast<entt::entity>(entityID) };
	if (!(engine->ecs.registry.any_of<T>(entity)))
		return nullptr;
	return &(engine->ecs.registry.get<T>(entity));
}

template<typename Type, typename ...Types>
bool Interface::ObtainPrimitiveDataFromScript(FieldData& fieldData, Object^ object)
{
	try {
		Type^ value = safe_cast<Type^>(object);
		fieldData.data = safe_cast<Type>(*value);
		return true;
	}
	catch(...){}
	if constexpr (sizeof...(Types) > 0)
		return ObtainPrimitiveDataFromScript<Types...>(fieldData, object);
	else
		return false;
}

template<typename Type, typename ...Types>
bool Interface::SetScriptPrimitiveFromNativeData(FieldData const& fieldData, Script^ script, System::Reflection::FieldInfo^ fieldInfo)
{
	if (std::holds_alternative<Type>(fieldData.data)) {
		Type value = std::get<Type>(fieldData.data);
		Type^ managedValue = safe_cast<Type^>(value);
		fieldInfo->SetValue(script, managedValue);
		return true;
	}
	if constexpr (sizeof...(Types) > 0)
		return SetScriptPrimitiveFromNativeData<Types...>(fieldData, script, fieldInfo);
	else
		return false;
}

template<typename Type, typename ...Types>
bool Interface::ObtainTypedResourceIDFromScript(FieldData& fieldData, Object^ object, System::Type^ originalType) {
		if (originalType == Type::typeid) {
		try {
			Type^ value = safe_cast<Type^>(object);

			// we default construct one..
			if (!value) {
				object = gcnew Type{ { INVALID_RESOURCE_ID } };
				value = safe_cast<Type^>(object);
			}
			fieldData.data = value->getId();
			return true;
		}
		catch (System::Exception^) {}
	}

	if constexpr (sizeof...(Types) > 0)
		return ObtainTypedResourceIDFromScript<Types...>(fieldData, object, originalType);
	else	
		return false;
}

template<typename Type, typename ...Types>
bool Interface::SetTypedResourceIDFromScript(FieldData const& fieldData, Script^ script, System::Reflection::FieldInfo^ fieldInfo) {
	try {
		// this step is just to check if the field info is of this Type..
		Type^ value = safe_cast<Type^>(fieldInfo->GetValue(script));
			
		using variantType = decltype(value->getId());
		variantType const& varantValue = std::get<variantType>(fieldData.data);

		value = gcnew Type{ { varantValue } };
		
		fieldInfo->SetValue(script, value);
	}
	catch (...) {}

	if constexpr (sizeof...(Types) > 0)
		return SetTypedResourceIDFromScript<Types...>(fieldData, script, fieldInfo);
	else 
		return false;
}

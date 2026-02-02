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
	return engine->ecs.registry.try_get<T>(entity);
}

template<typename Type, typename ...Types>
bool Interface::ObtainPrimitiveDataFromScript(serialized_field_type& fieldData, Object^ object)
{
	Type^ value = dynamic_cast<Type^>(object);
	if (!value) {
		if constexpr (sizeof...(Types) > 0)
			return ObtainPrimitiveDataFromScript<Types...>(fieldData, object);
		else
			return false;
	}
	fieldData = safe_cast<Type>(*value);
	return true;
}

template<typename Type, typename ...Types>
bool Interface::SetScriptPrimitiveFromNativeData(serialized_field_type const& fieldData, Object^% object)
{
	if (std::holds_alternative<Type>(fieldData)) {
		Type value = std::get<Type>(fieldData);
		Type^ managedValue = safe_cast<Type^>(value);
		object = managedValue;
		return true;
	}
	if constexpr (sizeof...(Types) > 0)
		return SetScriptPrimitiveFromNativeData<Types...>(fieldData, object);
	else
		return false;
}

template<typename Type, typename ...Types>
bool Interface::ObtainTypedResourceIDFromScript(serialized_field_type& fieldData, Object^ object, System::Type^ originalType) {
		if (originalType == Type::typeid) {
		try {
			Type^ value = safe_cast<Type^>(object);

			// we default construct one..
			if (!value) {
				object = gcnew Type{ { INVALID_RESOURCE_ID } };
				value = safe_cast<Type^>(object);
			}
			fieldData = value->getId();
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
bool Interface::SetTypedResourceIDFromScript(serialized_field_type const& fieldData, Object^% object) {
	Type^ value = dynamic_cast<Type^>(object);

	if (!value) {
		using variantType = decltype(value->getId());

		if (std::holds_alternative<variantType>(fieldData)) {
			// this step is just to check if the field info is of this Type..
			variantType const& varantValue = std::get<variantType>(fieldData);
			value = gcnew Type{ { varantValue } };
			object = value;
			return true;
		}
	}

	if constexpr (sizeof...(Types) > 0)
		return SetTypedResourceIDFromScript<Types...>(fieldData, object);
	else 
		return false;
}

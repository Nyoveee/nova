#include "ScriptingAPI.hxx"
#include "Engine/engine.h"
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

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
bool Interface::AppendPrimitiveData(FieldData& fieldData, Object^ object)
{
	try {
		Type^ value = safe_cast<Type^>(object);
		fieldData.second = safe_cast<Type>(*value);
		return true;
	}
	catch(...){}
	if constexpr (sizeof...(Types) > 0)
		return AppendPrimitiveData<Types...>(fieldData, object);
	else
		return false;
}
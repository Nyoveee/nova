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
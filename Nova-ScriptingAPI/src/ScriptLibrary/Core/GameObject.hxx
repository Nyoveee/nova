#pragma once
#include "ComponentAccessor.hxx"
#include "../Extensions/ManagedTypes.hxx"

#include <msclr/marshal_cppstd.h>
public ref class GameObject : ComponentAccessor
{
public:
	virtual System::String^ ToString() override sealed {
		EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(static_cast<entt::entity>(entityID));
		return entityData? msclr::interop::marshal_as<System::String^>(entityData->name.c_str()) : "";
	};
public:
	property Transform_^ transform{
		Transform_^ get() { return transformReference; };
	}
internal:
	Transform_^ transformReference;
};


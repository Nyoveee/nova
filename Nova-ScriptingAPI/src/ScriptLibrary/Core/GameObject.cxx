#include "GameObject.hxx"
#include "ScriptLibrary/Extensions/ManagedTypes.hxx"
#include <msclr/marshal_cppstd.h>
GameObject^ GameObject::GetReference(System::UInt32 p_entityID)
{
	GameObject^ newGameObject = gcnew GameObject();
	newGameObject->entityID = p_entityID;
	newGameObject->transformReference = newGameObject->getComponent<Transform_^>();
	return newGameObject;
}

System::String^ GameObject::ToString()
{
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(static_cast<entt::entity>(entityID));
	return entityData ? msclr::interop::marshal_as<System::String^>(entityData->name.c_str()) : "";
}

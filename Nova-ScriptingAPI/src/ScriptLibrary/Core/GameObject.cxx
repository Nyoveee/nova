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

GameObject^ GameObject::Find(System::String^ name)
{
	for (auto&& [entity, entityData] : Interface::engine->ecs.registry.view<EntityData>().each())
		if (entityData.name == msclr::interop::marshal_as<std::string>(name))
			return GetReference(static_cast<unsigned int>(entity));
	return nullptr;
}

array<GameObject^>^ GameObject::FindGameObjectsWithTag(System::String^ tag)
{
	System::Collections::Generic::List<GameObject^> gameObjects;
	for (auto&& [entity, entityData] : Interface::engine->ecs.registry.view<EntityData>().each())
		if (entityData.tag == msclr::interop::marshal_as<std::string>(tag))
			gameObjects.Add(GetReference(static_cast<unsigned int>(entity)));
	return gameObjects.ToArray();
}

GameObject^ GameObject::FindWithTag(System::String^ tag)
{
	for (auto&& [entity, entityData] : Interface::engine->ecs.registry.view<EntityData>().each())
		if (entityData.tag == msclr::interop::marshal_as<std::string>(tag))
			return GetReference(static_cast<unsigned int>(entity));
	return nullptr;
}

System::String^ GameObject::ToString()
{
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(static_cast<entt::entity>(entityID));
	return entityData ? msclr::interop::marshal_as<System::String^>(entityData->name.c_str()) : "";
}
GameObject^ GameObject::GetParent()
{
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(static_cast<entt::entity>(entityID));
	if (entityData->parent == entt::null)
		return nullptr;
	return GetReference(static_cast<unsigned int>(entityData->parent));
}

Transform_^ GameObject::transform::get() { return transformReference; };
System::String^ GameObject::tag::get() { return msclr::interop::marshal_as<System::String^>(Interface::getNativeComponent<EntityData>(entityID)->tag); }
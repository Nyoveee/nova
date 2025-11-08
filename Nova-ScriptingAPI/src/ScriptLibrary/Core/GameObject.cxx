#include "GameObject.hxx"
#include "ScriptLibrary/Extensions/ManagedTypes.hxx"
#include <msclr/marshal_cppstd.h>

// delegating ctor.
GameObject::GameObject(entt::entity entity) :
	GameObject { static_cast<unsigned>(entity) }
{}

GameObject::GameObject(System::UInt32 p_entityID) {
	entityID = p_entityID;
	transformReference = getComponent<Transform_^>();
}

GameObject^ GameObject::GetReference(System::UInt32 p_entityID)
{
	GameObject^ newGameObject = gcnew GameObject(p_entityID);
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

array<GameObject^>^ GameObject::GetChildren()
{
	System::Collections::Generic::List<GameObject^> children;
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(static_cast<entt::entity>(entityID));
	for (entt::entity child : entityData->children)
		children.Add(GetReference(static_cast<unsigned int>(child)));
	return children.ToArray();
}

System::UInt32 GameObject::GetId() {
	return entityID;
}

void GameObject::SetActive(bool active) {
	Interface::engine->ecs.setActive(static_cast<entt::entity>(entityID), active);
}

Transform_^ GameObject::transform::get() { return transformReference; };
System::String^ GameObject::tag::get() { return msclr::interop::marshal_as<System::String^>(Interface::getNativeComponent<EntityData>(entityID)->tag); }
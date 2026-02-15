#include "ComponentAccessor.hxx"
#include "API/ScriptingAPI.hxx"

// Generics behave just like a normal function, this definition is compiled through clr at compile time
// Specializations are created at runtime
// https://learn.microsoft.com/en-us/cpp/extensions/overview-of-generics-in-visual-cpp?view=msvc-170
generic<typename T> where T : IManagedComponent
T ComponentAccessor::getComponent() {
	// We try to load the component data from the entity itself
	T component = safe_cast<T>(System::Activator::CreateInstance(T::typeid));
	if (!component->LoadDetailsFromEntity(entityID))
		return T(); // Component will be garbage collected, return null
	return component;
}

generic<typename T> where T : Script
T ComponentAccessor::getScript() { return Interface::tryGetScriptReference<T>(entityID); }

// ======================================
// Game Object creation and deletion..
// ======================================

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab) {
	return Instantiate(prefab, Vector3::Zero(), Quaternion::Identity(), nullptr);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent) {
	return Instantiate(prefab, Vector3::Zero(), Quaternion::Identity(), parent);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent) {
	constexpr glm::quat identityQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
	return Instantiate(prefab, localPosition, gcnew Quaternion{ identityQuat }, parent);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quaternion^ localRotation, GameObject^ parent) {
	entt::entity prefabInstanceId = Interface::engine->prefabManager.instantiatePrefab(prefab->getId());

	// set parent, and local transform..
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(prefabInstanceId);
	Transform* transform = Interface::engine->ecs.registry.try_get<Transform>(prefabInstanceId);

	if (!entityData || !transform) {
		Logger::warn("{}Failed to instantiate entity?",GameObject(entityID).GetNameID());
		return nullptr;
	}

	if (parent && static_cast<entt::entity>(parent->entityID) != entt::null) {
		Interface::engine->ecs.setEntityParent(prefabInstanceId, static_cast<entt::entity>(parent->entityID), false);
	}

	transform->localPosition = localPosition->native();
	transform->localRotation = localRotation->native();
	Interface::recursivelyInitialiseEntity(prefabInstanceId);
	
	// yoinked from zhi wei
	GameObject^ newGameObject = gcnew GameObject(prefabInstanceId);

	return newGameObject;
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quaternion^ localRotation) {
	return Instantiate(prefab, localPosition, localRotation, nullptr);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition) {
	constexpr glm::quat identityQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
	return Instantiate(prefab, localPosition, gcnew Quaternion{ identityQuat }, nullptr);
}

void ComponentAccessor::Destroy(GameObject^ gameObject) {
	if (!gameObject || !Interface::engine->ecs.registry.valid(static_cast<entt::entity>(gameObject->entityID))) {
		Logger::error("{}Attempting to delete GameObject that doesn't exist",this->gameObject->GetNameID());
		return;
	}
	
	for each (GameObject ^ child in gameObject->GetChildren())
		Destroy(child);

	Interface::submitGameObjectDeleteRequest(gameObject->entityID);
	gameObject->entityID = static_cast<unsigned>(entt::null);
}

void ComponentAccessor::Invoke(Callback^ callback, float duration) {
	Interface::addTimeoutDelegate(gcnew TimeoutDelegate(callback, duration));
}
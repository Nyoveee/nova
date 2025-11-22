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
	return Instantiate(prefab, nullptr);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent) {
	constexpr glm::vec3 zeroPos = { 0.f, 0.f, 0.f };
	return Instantiate(prefab, gcnew Vector3{ zeroPos }, parent);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent) {
	constexpr glm::quat identityQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
	return Instantiate(prefab, localPosition, gcnew Quaternion{ identityQuat }, parent);
}

GameObject^ ComponentAccessor::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quaternion^ localRotation, GameObject^ parent) {
	entt::entity prefabInstanceId = Interface::engine->prefabManager.instantiatePrefab<ALL_COMPONENTS>(prefab->getId());

	// set parent, and local transform..
	EntityData* entityData = Interface::engine->ecs.registry.try_get<EntityData>(prefabInstanceId);
	Transform* transform = Interface::engine->ecs.registry.try_get<Transform>(prefabInstanceId);

	if (!entityData || !transform) {
		Logger::warn("Failed to instantiate entity?");
		return nullptr;
	}

	if (parent && static_cast<entt::entity>(parent->entityID) != entt::null) {
		Interface::engine->ecs.setEntityParent(prefabInstanceId, static_cast<entt::entity>(parent->entityID));
		transform->localPosition = localPosition->native();
		transform->localRotation = localRotation->native();
	}
	else {
		transform->position = localPosition->native();
		transform->rotation = localRotation->native();
	}

	//if have NavMeshAgent, init navmeshagent
	NavMeshAgent* agent = Interface::engine->ecs.registry.try_get<NavMeshAgent>(prefabInstanceId);

	if (agent)
	Interface::engine->navigationSystem.InstantiateAgentsToSystem(prefabInstanceId, transform , agent);

	// initialise animator..
	Animator* animator = Interface::engine->ecs.registry.try_get<Animator>(prefabInstanceId);

	if (animator)
		Interface::engine->animationSystem.initialiseAnimator(*animator);

	// initialise all scripts..
	Scripts* scripts = Interface::engine->ecs.registry.try_get<Scripts>(prefabInstanceId);

	if (scripts) {
		System::Collections::Generic::List<Script^>^ list = gcnew System::Collections::Generic::List<Script^>();
		// Instantiate these scripts and call init..
		for (auto&& scriptData : scripts->scriptDatas) {
			Interface::ScriptID scriptId = static_cast<System::UInt64>(scriptData.scriptId);
			Script^ script = Interface::delayedAddEntityScript(static_cast<System::UInt32>(prefabInstanceId), scriptId);

			for (auto&& fieldData : scriptData.fields)
				Interface::setFieldData(script, fieldData);
			list->Add(script);
		}
		for each (Script ^ script in list)
			Interface::initializeScript(script);
	}

	// yoinked from zhi wei
	GameObject^ newGameObject = gcnew GameObject(prefabInstanceId);

	return newGameObject;
}

void ComponentAccessor::Destroy(GameObject^ gameObject) {
	Interface::submitGameObjectDeleteRequest(gameObject->entityID);
}

void ComponentAccessor::Invoke(Callback^ callback, float duration) {
	Interface::addTimeoutDelegate(gcnew TimeoutDelegate(callback, duration));
}
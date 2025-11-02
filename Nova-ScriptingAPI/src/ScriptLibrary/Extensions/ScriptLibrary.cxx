#include "ScriptLibrary.hxx"

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab) {
	return Instantiate(prefab, nullptr);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent) {
	constexpr glm::vec3 zeroPos = { 0.f, 0.f, 0.f };
	return Instantiate(prefab, gcnew Vector3{ zeroPos }, parent);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent) {
	constexpr glm::quat identityQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
	return Instantiate(prefab, localPosition, gcnew Quartenion{ identityQuat }, parent);
}

GameObject^ ObjectAPI::Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quartenion^ localRotation, GameObject^ parent) {
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

	// initialise all scripts..
	Scripts* scripts = Interface::engine->ecs.registry.try_get<Scripts>(prefabInstanceId);

	if (scripts) {
		// Instantiate these scripts and call init..
		for (auto&& scriptData : scripts->scriptDatas) {
			Interface::addEntityScript(static_cast<unsigned>(prefabInstanceId), static_cast<unsigned long long>(scriptData.scriptId));
			Interface::initializeScript(static_cast<unsigned>(prefabInstanceId), static_cast<unsigned long long>(scriptData.scriptId));
		}
	}

	// yoinked from zhi wei
	GameObject^ newGameObject = gcnew GameObject();
	newGameObject->entityID = static_cast<unsigned>(prefabInstanceId);
	newGameObject->transformReference = newGameObject->getComponent<Transform_^>();

	return newGameObject;
}

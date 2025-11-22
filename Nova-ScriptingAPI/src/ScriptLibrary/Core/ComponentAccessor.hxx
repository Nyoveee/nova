#pragma once

#include "API/IManagedComponent.hxx"
#include "API/TimeoutDelegate.hxx"

ref class Script;
ref class GameObject;

value struct Vector3;
value struct Quartenion;

namespace ScriptingAPI {
	ref class Prefab;
};

public ref class ComponentAccessor{
public:
	// Get reference to the component
	generic<typename T> where T : IManagedComponent
	T getComponent();
	generic<typename T> where T : Script
	T getScript();

public:
	static void Invoke(Callback^ callback, float duration);

	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quartenion^ localRotation, GameObject^ parent);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quartenion^ localRotation);
	static GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition);

	static void Destroy(GameObject^ gameObject);

internal:
	System::UInt32 entityID;
};


#pragma once

#include "API/IManagedComponent.hxx"
#include "API/TimeoutDelegate.hxx"

ref class Script;
ref class GameObject;

value struct Vector3;
value struct Quaternion;

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
	void Invoke(Callback^ callback, float duration);

	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab);
	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, GameObject^ parent);
	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, GameObject^ parent);
	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quaternion^ localRotation, GameObject^ parent);
	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition, Quaternion^ localRotation);
	GameObject^ Instantiate(ScriptingAPI::Prefab^ prefab, Vector3^ localPosition);

	void Destroy(GameObject^ gameObject);
internal:
	GameObject^ _gameObject;
	System::UInt32 entityID;

public:
	property GameObject^ gameObject {
		GameObject^ get() { return _gameObject; };
	}

};


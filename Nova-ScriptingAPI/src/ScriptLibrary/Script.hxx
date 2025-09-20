// Base C# Script
// From this, scripts are considered components
// Implementing might be a little bit messy though
// https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html 

#pragma once
#include "API/IManagedComponent.hxx"
[System::AttributeUsage(System::AttributeTargets::Field)]
public ref class SerializableField : System::Attribute {};

public ref class Script abstract
{
public:
	// Get reference to the component
	generic<typename T> where T : IManagedComponent
	T getComponent();
	generic<typename T> where T : Script
	T getScript();
internal:
	// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
	// These also includes exception handling for scripts
	void callInit();
	void callUpdate();
	void callExit();
protected:
	virtual void init() {};
	virtual void update() {};
	virtual void exit() {};
internal:
	System::UInt32 entityID;
};

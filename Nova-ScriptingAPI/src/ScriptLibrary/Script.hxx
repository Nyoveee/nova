// Base C# Script
// From this, scripts are considered components
// Implementing might be a little bit messy though
// https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html 

#pragma once
#include "IManagedComponent.hxx"
public ref class Script abstract : IManagedComponent
{
public:
	// Get reference to the component
	generic<typename Component> where Component : IManagedComponent
	Component getComponent();
internal:
	// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
	// These also includes exception handling for scripts
	void callInit();
	void callUpdate();
	void callExit();
	virtual bool exist(System::UInt32) override sealed;
protected:
	virtual void init() {};
	virtual void update() {};
	virtual void exit() {};
};

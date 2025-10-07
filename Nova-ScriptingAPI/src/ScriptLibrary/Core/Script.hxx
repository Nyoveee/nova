// Base C# Script
// From this, scripts are considered components
// Implementing might be a little bit messy though
// https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html 

#pragma once

#include "ComponentAccessor.hxx"
#include "ScriptAttributes.hxx"
#include "GameObject.hxx"
public ref class Script abstract : ComponentAccessor
{
internal:
	// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
	// These also includes exception handling for scripts
	void callInit();
	void callUpdate();
	void callExit();

	void callOnCollisionEnter(unsigned otherEntityID);

protected:
	virtual void init() {};
	virtual void update() {};
	virtual void exit() {};

	virtual void onCollisionEnter([[maybe_unused]] GameObject^ other) {};
protected:
	GameObject^ gameObject;
};

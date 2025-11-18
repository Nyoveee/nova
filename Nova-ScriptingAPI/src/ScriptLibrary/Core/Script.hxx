// Base C# Script
// From this, scripts are considered components
// Implementing might be a little bit messy though
// https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html 

#pragma once

#include "ComponentAccessor.hxx"
#include "ScriptAttributes.hxx"
#include "GameObject.hxx"
#include "ScriptLibrary\Extensions\ScriptLibrary.hxx"

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
	// Subscribing to input manager..
	void MapKey(Key key, EventCallback^ pressCallback) {
		scriptObserverIds.Add(Input::MapKey(key, pressCallback));
	}

	void MapKey(Key key, EventCallback^ pressCallback, EventCallback^ releaseCallback) {
		scriptObserverIds.Add(Input::MapKey(key, pressCallback, releaseCallback));
	}

	void MouseMoveCallback(MouseEventCallback^ callback) {
		mouseMoveObserverIds.Add(Input::MouseMoveCallback(callback));
	}

	void ScrollCallback(ScrollEventCallback^ callback) {
		mouseScrollObserverIds.Add(Input::ScrollCallback(callback));
	}

internal:
	GameObject^ _gameObject;

public:
	property GameObject^ gameObject {
		GameObject^ get() { return _gameObject; };
	}

internal:
	// Each script is aware of what functions it subscribes to with the Input Manager. 
	// When the game object holding this instance of the script is destroyed, we need to unsubscribe.
	System::Collections::Generic::List<std::size_t> scriptObserverIds;
	System::Collections::Generic::List<std::size_t> mouseMoveObserverIds;
	System::Collections::Generic::List<std::size_t> mouseScrollObserverIds;
};

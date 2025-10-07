#pragma once
#include "API/IManagedComponent.hxx"
ref class Script;
public ref class ComponentAccessor{
public:
	// Get reference to the component
	generic<typename T> where T : IManagedComponent
	T getComponent();
	generic<typename T> where T : Script
	T getScript();
internal:
	System::UInt32 entityID;
};


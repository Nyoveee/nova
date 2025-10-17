#pragma once
#include "ComponentAccessor.hxx"
ref class Transform_;
public ref class GameObject : ComponentAccessor
{
public:
	static GameObject^ GetReference(System::UInt32 p_entityID);
public:
	virtual System::String^ ToString() override sealed;
public:
	property Transform_^ transform{
		Transform_^ get() { return transformReference; };
	}
internal:
	Transform_^ transformReference;
};


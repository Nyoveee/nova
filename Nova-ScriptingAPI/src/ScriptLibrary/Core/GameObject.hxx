#pragma once
#include "ComponentAccessor.hxx"
ref class Transform_;
public ref class GameObject : ComponentAccessor
{
internal:
	static GameObject^ GetReference(System::UInt32 p_entityID);
public:
	static GameObject^ Find(System::String^ name);
	static array<GameObject^>^ FindGameObjectsWithTag(System::String^ tag);
	static GameObject^ FindWithTag(System::String^ tag);
public:
	virtual System::String^ ToString() override sealed;
public:
	property Transform_^ transform{
		Transform_^ get() { return transformReference; };
	}
internal:
	Transform_^ transformReference;
};


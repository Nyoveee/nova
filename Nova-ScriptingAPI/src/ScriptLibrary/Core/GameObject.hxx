#pragma once

#include "ComponentAccessor.hxx"

#include <entt/entt.hpp>

ref class Transform_;
public ref class GameObject : ComponentAccessor
{
public:
	// Constructs a game object given an entity id..
	GameObject();
	GameObject(System::UInt32 p_entityID);
	GameObject(entt::entity entity);

	static GameObject^ GetReference(System::UInt32 p_entityID);

public:
	static GameObject^ Find(System::String^ name);
	static array<GameObject^>^ FindGameObjectsWithTag(System::String^ tag);
	static GameObject^ FindWithTag(System::String^ tag);

public:
	virtual System::String^ ToString() override sealed;

	GameObject^ GetParent();
	array<GameObject^>^ GetChildren();
	System::UInt32 GetId();

	void SetActive(bool active);
	bool IsActive();

public:
	property Transform_^ transform{
		Transform_^ get();
	}
	property System::String^ tag {
		System::String^ get();
	}

public:
	static bool operator==(GameObject^ lhs, GameObject^ rhs) {
		// zhi wei can overwrite this this is temp.
		if (!lhs || !rhs) {
			return false;
		}

		return lhs->entityID == rhs->entityID;
	}

internal:
	Transform_^ transformReference;
};


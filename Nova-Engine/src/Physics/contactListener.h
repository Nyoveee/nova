#pragma once

#include <iostream>
#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>

class PhysicsManager;

// See: ContactListener

class NovaContactListener : public JPH::ContactListener {
public:
	NovaContactListener(PhysicsManager& physicsManager);

public:
	JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) final;

	void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings);

	void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings);

	void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) final;

private:
	 PhysicsManager& physicsManager;
};
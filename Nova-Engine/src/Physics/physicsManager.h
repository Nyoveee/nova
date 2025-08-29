#pragma once

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation. (nah)
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "joltPhysicsInterface.h"

class PhysicsManager {
public:
	PhysicsManager();

	~PhysicsManager();
	PhysicsManager(PhysicsManager const& other)				= delete;
	PhysicsManager(PhysicsManager&& other)					= delete;
	PhysicsManager& operator=(PhysicsManager const& other)	= delete;
	PhysicsManager& operator=(PhysicsManager&& other)		= delete;

public:
	void update(float dt);

private:
	void createPrimitiveShapes();

private:
	// we use a placeholder data member to invoke certain functions before the construction of the following Jolt data members.
	int placeholder;

private:
	JPH::PhysicsSystem physicsSystem;

	JPH::TempAllocatorImpl temp_allocator;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	JPH::JobSystemThreadPool job_system;

	JPH::BodyInterface& bodyInterface;

private:
	// We let this physics manager owns some basic primitive shapes.
	JPH::Ref<JPH::Shape> box;
	JPH::Ref<JPH::Shape> sphere;
};
#pragma once

#include <glm/vec3.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <mutex>
#include <utility>
#include <queue>
#include <optional>
#include <stack>

class Engine;

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation. (nah)
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/CastResult.h>

#include "joltPhysicsInterface.h"
#include "debugRenderer.h"
#include "contactListener.h"
#include "ECS/Events.h"

#include "export.h"
#include "physics.h"

class PhysicsManager {
public:
	PhysicsManager(Engine& engine);

	~PhysicsManager();
	PhysicsManager(PhysicsManager const& other)				= delete;
	PhysicsManager(PhysicsManager&& other)					= delete;
	PhysicsManager& operator=(PhysicsManager const& other)	= delete;
	PhysicsManager& operator=(PhysicsManager&& other)		= delete;

public:
	void simulationInitialise();
	void systemInitialise();
	void clear();
	void updatePhysics(float dt);
	void debugRender();
	void transformUpdateListener(TransformUpdateEvent const& event);
	void updateTransformBodies();

	// Nova Collision Listener submits all collision event here..
	void submitCollision(entt::entity entityOne, entt::entity entityTwo);
	
	ENGINE_DLL_API PhysicsRay getRayFromMouse() const;
	ENGINE_DLL_API std::optional<PhysicsRayCastResult> rayCast(PhysicsRay ray, float maxDistance);

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

	// Our own debug renderer..
	DebugRenderer debugRenderer;
	NovaContactListener contactListener;

	entt::registry& registry;
	Engine& engine;

private:
	// We let this physics manager owns some basic primitive shapes.
	JPH::Ref<JPH::Shape> box;
	JPH::Ref<JPH::Shape> sphere;

	std::vector<JPH::BodyID> createdBodies;

	//on update transform 
	std::stack<entt::entity> transformUpdateStack; //when transform is updated push the id on stack. updateTransformBodies read from stack and update transform

	// we queue entities on collision for thread safety.
	std::queue<std::pair<entt::entity, entt::entity>> onCollision;
	std::mutex onCollisionMutex;
};
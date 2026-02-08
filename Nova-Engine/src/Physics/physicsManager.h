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
// recently figured out how to access motion properties https://github.com/jrouwe/JoltPhysics/discussions/1130
//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

#include "joltPhysicsInterface.h"
#include "debugRenderer.h"
#include "contactListener.h"
#include "ECS/Events.h"

#include "export.h"
#include "physics.h"
#include "component.h"


struct Transform;
struct Rigidbody;

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
	void resetPhysicsState();
	void transformUpdateListener(TransformUpdateEvent const& event);
	void updateTransformBodies();

	//add to system and remove from system
	void addBodiesToSystem(entt::registry&, entt::entity entityID);
	void removeBodiesFromSystem(entt::registry&, entt::entity entityID);
	bool isBodyAlive(JPH::BodyID id); //check if a body is alive

	// Nova Collision Listener submits all collision event here..
	void submitCollisionEnter(entt::entity entityOne, entt::entity entityTwo);
	void submitCollisionExit(entt::entity entityOne, entt::entity entityTwo);
	
public:
	// These interfaces are invoked by C# scripting..
	ENGINE_DLL_API void SetPhysicsLayer(Rigidbody& rigidbody, Rigidbody::Layer physicsLayer);

	ENGINE_DLL_API PhysicsRay getRayFromMouse() const;
	ENGINE_DLL_API std::optional<PhysicsRayCastResult> rayCast(PhysicsRay ray, float maxDistance, std::vector<entt::entity> const& ignoredEntities = {});
	ENGINE_DLL_API std::optional<PhysicsRayCastResult> rayCast(PhysicsRay ray, float maxDistance, std::vector<uint8_t> const& layerMask = {});

	ENGINE_DLL_API void addForce(Rigidbody const& rigidbody, glm::vec3 forceVector);
	ENGINE_DLL_API void addImpulse(Rigidbody const& rigidbody, glm::vec3 forceVector);

	ENGINE_DLL_API void setVelocity(Rigidbody& rigidbody, glm::vec3 velocity);
	ENGINE_DLL_API void setVelocityLimits(Rigidbody& rigidbody, float maxVelocity);
	ENGINE_DLL_API std::optional<float> getVelocityLimits(Rigidbody& rigidbody);
	ENGINE_DLL_API void setAngularVelocity(Rigidbody& rigidbody, glm::vec3 velocity);
	ENGINE_DLL_API void setMaxAngularVelocityLimits(Rigidbody& rigidbody, float maxVelocity);
	ENGINE_DLL_API std::optional<float> getAngularVelocityLimits(Rigidbody& rigidbody);

	ENGINE_DLL_API void setLinearDamping(Rigidbody& rigidbody, float dampingValue);
	ENGINE_DLL_API std::optional<float> getLinearDamping(Rigidbody& rigidbody);
	ENGINE_DLL_API void setAngularDamping(Rigidbody& rigidbody, float dampingValue);
	ENGINE_DLL_API std::optional<float> getAngularDamping(Rigidbody& rigidbody);

	ENGINE_DLL_API void setRotation(Rigidbody& rigidbody, glm::quat quaternion);

	ENGINE_DLL_API void setGravity(float value);
	ENGINE_DLL_API void setGravityFactor(Rigidbody& rigidbody, float value);

	ENGINE_DLL_API void setMass(Rigidbody& rigidbody, float value);
	ENGINE_DLL_API float getMass(Rigidbody& rigidbody);

private:
	void createPrimitiveShapes();
	void createBody(entt::entity const& entityID);
	bool hasRequiredPhysicsComponents(entt::entity const& entityID);

	// Given entity and his rigidbody, we recreate a shape according to what colliders it has.
	JPH::ScaledShape* recreateScaledShape(entt::entity entity, Transform const& transform, Rigidbody& rigidbody);

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

	float gravityStrength = 5.0f;

private:
	friend class NovaContactListener;

	// We let this physics manager owns some basic primitive shapes.
	JPH::Ref<JPH::Shape> box;
	JPH::Ref<JPH::Shape> sphere;
	JPH::Ref<JPH::Shape> capsule;

	std::vector<JPH::BodyID> createdBodies;

	// we keep track of bodies, manaully setting their angular velocity to 0.
	std::vector<JPH::BodyID> nonRotatableBodies;

	//on update transform 
	std::stack<entt::entity> transformUpdateStack; //when transform is updated push the id on stack. updateTransformBodies read from stack and update transform

	// we queue entities on collision for thread safety.
	std::queue<std::pair<entt::entity, entt::entity>> onCollisionEnter;
	// we queue entities on collision for thread safety.
	std::queue<std::pair<entt::entity, entt::entity>> onCollisionExit;
	std::mutex onCollisionMutex;
};
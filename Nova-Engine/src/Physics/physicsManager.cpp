#include "Engine/engine.h"
#include "physicsManager.h"
#include "component.h"
#include "ECS/ECS.h"
#include "Engine/window.h"
#include "ECS/Events.h"

#include "Profiling.h"

#include <iostream>
#include <cstdarg>

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace {
	// Callback for traces, connect this to your own trace function if you have one
	void TraceImpl(const char* inFMT, ...) {
		// Format the message
		std::va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		// Print to the TTY
		std::cout << buffer << '\n';
	}

	// Callback for asserts, connect this to your own assert handler if you have one
	bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, unsigned int inLine) {
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << '\n';

		// Breakpoint
		return true;
	};

	JPH::RVec3Arg toJPHVec3(glm::vec3 const& glmVec3) {
		return { glmVec3.x, glmVec3.y, glmVec3.z };
	}

	glm::vec3 toGlmVec3(JPH::RVec3Arg const& jphVec3) {
		return { jphVec3.GetX(), jphVec3.GetY(), jphVec3.GetZ() };
	}

	JPH::Quat toJPHQuat(glm::quat const& glmQuat) {
		return { glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w };
	}

	glm::quat toGlmQuat(JPH::Quat const& jphQuat) {
		// because glm built different.
		return { jphQuat.GetW(), jphQuat.GetX(), jphQuat.GetY(), jphQuat.GetZ() };
	}
}

// We need a temp allocator for temporary allocations during the physics update. We're
// pre-allocating 10 MB to avoid having to do allocations during the physics update.
// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
// malloc / free.
constexpr std::size_t TEMPORARY_MEMORY = 10 * 1024 * 1024;

constexpr unsigned int maxPhysicsJobs = 2048;
constexpr unsigned int maxPhysicsBarriers = 8;

// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
constexpr unsigned int maxBodies = 1024;

// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
constexpr unsigned int numBodyMutexes = 0;

// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
constexpr unsigned int maxBodyPairs = 1024;

// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
constexpr unsigned int maxContactConstraints = 1024;

PhysicsManager::PhysicsManager(Engine& engine) :
	// we use a placeholder to invoke a function before constructing the rest of the data member.
	placeholder		{ [&](){ 
		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();
		return 0;
	}() },

	temp_allocator	{ TEMPORARY_MEMORY },
	job_system		{ maxPhysicsJobs, maxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() / 2U) },

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	// @ IF SOMETHING SEGFAULTS HERE ITS PROBABLY CAUSE I CALL .GETBODYINTERFACE BEFORE .INIT.
	bodyInterface	{ physicsSystem.GetBodyInterface() },
	debugRenderer	{ engine.renderer },
	registry		{ engine.ecs.registry },
	engine			{ engine },
	contactListener	{ *this }
{
	// Install trace and assert callbacks
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

	// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
	// It is not directly used in this example but still required.
	JPH::Factory::sInstance = new JPH::Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	JPH::RegisterTypes();

	// Now we can create the actual physics system.
	physicsSystem.Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
	
	//set transformUpdateListener to listen to transfrom events from transform system
	engine.ecs.systemEventDispatcher.sink<TransformUpdateEvent>().connect<&PhysicsManager::transformUpdateListener>(*this);

	// We create some primitive shapes to be shared.
	createPrimitiveShapes();

#if 0
	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyBodyActivationListener body_activation_listener;
	physics_system.SetBodyActivationListener(&body_activation_listener);


#endif
	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	physicsSystem.SetContactListener(&contactListener);

}

PhysicsManager::~PhysicsManager() {
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
}

void PhysicsManager::simulationInitialise() {
	//for (auto&& [entityId, transform, rigidbody, boxCollider] : registry.view<Transform, Rigidbody, BoxCollider>().each()) {

	//	// Create and add body based on entity's component.
	//	JPH::ObjectLayer layer = static_cast<JPH::ObjectLayer>(rigidbody.layer);

	//	JPH::BodyCreationSettings bodySettings{
	//		new JPH::ScaledShape(box, toJPHVec3(transform.scale * boxCollider.scaleMultiplier)),	// scaled shape
	//		toJPHVec3(transform.position),															// position
	//		toJPHQuat(transform.rotation),															// rotation (in quartenions)
	//		rigidbody.motionType,																	// motion type
	//		layer																					// in which layer?
	//	};

	//	JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(
	//		bodySettings, 
	//		JPH::EActivation::Activate //should always be active imo lmao
	//	);

	//	bodyInterface.SetUserData(bodyId, static_cast<unsigned>(entityId));
	//	bodyInterface.SetLinearVelocity(bodyId, toJPHVec3(rigidbody.initialVelocity));
	//	createdBodies.push_back(bodyId);

	//	rigidbody.bodyId = bodyId;
	//}

	//physicsSystem.OptimizeBroadPhase();


	//just set active is alright liao i think
	for (auto const& bodyID : createdBodies)
	{
		bodyInterface.ActivateBody(bodyID);
	}

}

void PhysicsManager::systemInitialise()
{
	for (auto&& [entityId, transform, rigidbody, boxCollider] : registry.view<Transform, Rigidbody, BoxCollider>().each()) {
		// Create and add body based on entity's component.
		JPH::ObjectLayer layer = static_cast<JPH::ObjectLayer>(rigidbody.layer);

		JPH::BodyCreationSettings bodySettings{
			new JPH::ScaledShape(box, toJPHVec3(transform.scale * boxCollider.scaleMultiplier)),	// scaled shape
			toJPHVec3(transform.position),															// position
			toJPHQuat(transform.rotation),															// rotation (in quartenions)
			rigidbody.motionType,																	// motion type
			layer																					// in which layer?
		};

		JPH::EActivation activationType = engine.isInSimulationMode() ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;

		JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(
			bodySettings,
		    activationType //should always be inactive unless its simulation step
		);

		bodyInterface.SetUserData(bodyId, static_cast<unsigned>(entityId));
		bodyInterface.SetLinearVelocity(bodyId, toJPHVec3(rigidbody.initialVelocity));
		createdBodies.push_back(bodyId);

		rigidbody.bodyId = bodyId;
	}

	physicsSystem.OptimizeBroadPhase();


}

void PhysicsManager::clear() {
	bodyInterface.RemoveBodies(createdBodies.data(), static_cast<int>(createdBodies.size()));
	bodyInterface.DestroyBodies(createdBodies.data(), static_cast<int>(createdBodies.size()));
	createdBodies.clear();

	for (auto&& [entityId, transform, rigidbody] : registry.view<Transform, Rigidbody>().each()) {
		rigidbody.bodyId = JPH::BodyID{}; // default constructed bodyID is invalid.
	}
}

void PhysicsManager::updatePhysics(float dt) {
	ZoneScoped;

	// =============================================================
	// 1. Update all physics body to the current object's transform.
	// @TODO: Don't update every frame! Only update when there is a change in transform. Ray: I tried to refactor this part
	// =============================================================
	//for (auto&& [entityId, transform, rigidbody] : registry.view<Transform, Rigidbody>().each()) {
	//	if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
	//		continue;
	//	}

	//	// our engine doesnt move static objects.
	//	if (rigidbody.motionType == JPH::EMotionType::Static) {
	//		continue;
	//	}

	//	bodyInterface.SetPositionAndRotation(rigidbody.bodyId, toJPHVec3(transform.position), toJPHQuat(transform.rotation), JPH::EActivation::Activate);
	//}
	//Update transform bodies have been moved outside to another function

	// run physics simulation.
	physicsSystem.Update(dt, 1, &temp_allocator, &job_system);	

	// =============================================================
	// 2. Update all object transform after running physics simulation.
	// @TODO: Don't update every frame! Only update when there is a change in transform.
	// =============================================================

	for (auto&& [entityId, transform, rigidbody] : registry.view<Transform, Rigidbody>().each()) {
		if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
			continue;
		}

		if (rigidbody.motionType == JPH::EMotionType::Static) {
			continue;
		}

		JPH::Vec3 pos;
		JPH::Quat rotation;

		bodyInterface.GetPositionAndRotation(rigidbody.bodyId, pos, rotation);
		transform.position = toGlmVec3(pos);
		transform.rotation = toGlmQuat(rotation);
	}

	// =============================================================
	// 3. Handle collision request..
	// =============================================================

	std::lock_guard lock{ onCollisionMutex };

	while (onCollision.size()) {
		auto [entityOne, entityTwo] = onCollision.front();
		onCollision.pop();
		
		engine.scriptingAPIManager.onCollisionEnter(entityOne, entityTwo);
	}
}

void PhysicsManager::debugRender() {

	//i think the best way to handle debugRender is that it is independent of whether the simulation is running. If the simulation is running
	// it is assumed that physics update handles transformation changes else look at transform and draw it.
	constexpr JPH::BodyManager::DrawSettings debugDrawSettings {};
	physicsSystem.DrawBodies(debugDrawSettings, &debugRenderer);
}

void PhysicsManager::transformUpdateListener(TransformUpdateEvent const& event)
{
	auto&& [rigidbody, boxCollider] = registry.try_get<Rigidbody, BoxCollider>(event.entityID);
	if (rigidbody == nullptr)
	{
		return;

	}
	
	//later on can have many types of collider as long as have one can.
	if (boxCollider == nullptr)
	{
		return;
	}

	transformUpdateStack.push(event.entityID);
}

void PhysicsManager::updateTransformBodies()
{
	//Look at transformUpdateListener, is filled from transform system
	while (!transformUpdateStack.empty())
	{
		entt::entity entity = transformUpdateStack.top();
		transformUpdateStack.pop();

		auto&& [transform , rigidbody ] = registry.get<Transform, Rigidbody>(entity);

		if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
			continue;
		}

		// our engine doesnt move static objects.
		if (rigidbody.motionType == JPH::EMotionType::Static) {
			continue;
		}

		bodyInterface.SetPositionAndRotation(rigidbody.bodyId, toJPHVec3(transform.position), toJPHQuat(transform.rotation), JPH::EActivation::Activate);
	}
}

void PhysicsManager::createPrimitiveShapes() {
	// ===========================================
	// 1. Constructing a box shape.
	// ===========================================
	JPH::BoxShapeSettings boxSettings { JPH::Vec3{ 1.f, 1.f, 1.f } };
	boxSettings.SetEmbedded();	// box settings is allocated on the stack, and this class is actually a smart pointer that does reference counting 
								// (for some reason) so we have to disable it

	box = boxSettings.Create().Get(); // hehe i ignore error.

	// ===========================================
	// 2. Constructing a sphere shape.
	// ===========================================
	JPH::SphereShapeSettings sphereSettings { 0.5f };
	sphereSettings.SetEmbedded(); // whatever i just yapped at the top

	sphere = sphereSettings.Create().Get();
}

void PhysicsManager::submitCollision(entt::entity entityOne, entt::entity entityTwo) {
	std::lock_guard lock{ onCollisionMutex };
	onCollision.push({ entityOne, entityTwo });
}

PhysicsRay PhysicsManager::getRayFromMouse() const {
	glm::vec3 farClipPos = { engine.window.getClipSpacePos(), 1.f };
	glm::vec3 nearClipPos = { farClipPos.x, farClipPos.y, -1.f };

	glm::vec3 farWorldPos = engine.renderer.getCamera().clipToWorldSpace(farClipPos);
	glm::vec3 nearWorldPos = engine.renderer.getCamera().clipToWorldSpace(nearClipPos);

	glm::vec3 raycastDirection = glm::normalize(farWorldPos - nearWorldPos);

	return { nearWorldPos, raycastDirection };
}

std::optional<PhysicsRayCastResult> PhysicsManager::rayCast(PhysicsRay ray, float maxDistance) {
	auto&& narrowPhaseQuery = physicsSystem.GetNarrowPhaseQuery();
	
	JPH::RayCastResult rayCastResult;
	glm::vec3 distanceVector = glm::normalize(ray.direction) * maxDistance;

	if (narrowPhaseQuery.CastRay(JPH::RRayCast{ toJPHVec3(ray.origin), toJPHVec3(distanceVector) }, rayCastResult)) {
		JPH::BodyID bodyId = rayCastResult.mBodyID;

		entt::entity entity = static_cast<entt::entity>(bodyInterface.GetUserData(bodyId));
		glm::vec3 collisionPoint = ray.origin + distanceVector * rayCastResult.mFraction;
	
		return PhysicsRayCastResult{ entity, collisionPoint };
	}

	return std::nullopt;
}
